/*
* Copyright I3D Robotics Ltd, 2020
* Author: Ben Knight
*/

#include "camerabasler.h"

CameraBasler::CameraBasler(QObject *parent) : QObject(parent)
{
    Pylon::PylonInitialize();
}

void CameraBasler::assignThread(QThread *thread)
{
    moveToThread(thread);
    connect(this, SIGNAL(finished()), thread, SLOT(quit()));
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}

bool CameraBasler::isAvailable() { return camera->IsGrabbing(); }

void CameraBasler::close() {
    try
    {
        if (camera){
            camera->StopGrabbing();
            camera->Close();
        }
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
    }
}

bool CameraBasler::initCamera(std::string camera_name,int binning)
{
    try
    {
        // Create an instant camera object with the camera device found first.
        Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();

        Pylon::DeviceInfoList_t devices;
        tlFactory.EnumerateDevices(devices);

        Pylon::CInstantCameraArray cameras(devices.size());
        Pylon::CDeviceInfo info;

        bool cameraFind = false;
        for (size_t i = 0; i < cameras.GetSize(); ++i)
        {
            cameras[i].Attach(tlFactory.CreateDevice(devices[i]));

            if (cameras[i].GetDeviceInfo().GetSerialNumber() == camera_name.c_str())
            {
                info = cameras[i].GetDeviceInfo();
                cameraFind = true;
                break;
            }
        }

        if (!cameraFind){
            std::cerr << "Failed to find camera with name: " << camera_name << std::endl;
            return false;
        }

        camera_device = Pylon::CTlFactory::GetInstance().CreateDevice(info);
        camera = new Pylon::CInstantCamera(camera_device);

        setBinning(binning);

        // Print the model name of the camera.
        std::cout << "Using device " << camera->GetDeviceInfo().GetModelName() << std::endl;

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera->MaxNumBuffer = 5;

        formatConverter.OutputPixelFormat = Pylon::PixelType_Mono8;
        formatConverter.OutputBitAlignment = Pylon::OutputBitAlignment_MsbAligned;

        // Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a default configuration which
        // sets up free-running continuous acquisition.
        camera->StartGrabbing();

        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

void CameraBasler::getImageSize(int &image_width, int &image_height,
                                cv::Size &image_size)
{
    try
    {
        camera->Open();
        image_width = Pylon::CIntegerParameter(camera->GetNodeMap(), "Width").GetValue();
        image_height = Pylon::CIntegerParameter(camera->GetNodeMap(), "Height").GetValue();
        image_size = cv::Size(image_height,image_width);
        //camera->Close();
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred whilst getting camera image size." << std::endl
                  << e.GetDescription() << std::endl;
    }
}

bool CameraBasler::setMaximumResolution(void)
{
    //TODO: set resolution
    return true;
}

bool CameraBasler::setFrame16(void)
{
    //TODO: set camera to 16bit
    return true;
}

bool CameraBasler::setFrame8(void)
{
    //TODO: set camera to 8bit
    return true;
}

bool CameraBasler::setPacketSize(int packetSize)
{
    try
    {
        // convert from seconds to milliseconds
        camera->Open();
        Pylon::CIntegerParameter(camera->GetNodeMap(), "GevSCPSPacketSize").SetValue(packetSize);
        //camera->Close();
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool CameraBasler::setInterPacketDelay(int interPacketDelay)
{
    try
    {
        // convert from seconds to milliseconds
        camera->Open();
        Pylon::CIntegerParameter(camera->GetNodeMap(), "GevSCPD").SetValue(interPacketDelay);
        //camera->Close();
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool CameraBasler::setBinning(int binning)
{
    try
    {
        // convert from seconds to milliseconds
        camera->Open();
        Pylon::CEnumParameter(camera->GetNodeMap(), "BinningHorizontalMode").FromString("Average");
        Pylon::CIntegerParameter(camera->GetNodeMap(), "BinningHorizontal").SetValue(binning);
        Pylon::CEnumParameter(camera->GetNodeMap(), "BinningVerticalMode").FromString("Average");
        Pylon::CIntegerParameter(camera->GetNodeMap(), "BinningVertical").SetValue(binning);
        //camera->Close();
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool CameraBasler::setExposure(double exposure)
{
    try
    {
        // convert from seconds to milliseconds
        int exposure_i = exposure * 10000;
        qDebug() << "Setting exposure..." << exposure_i;
        camera->Open();
        Pylon::CIntegerParameter(camera->GetNodeMap(), "ExposureTimeRaw").SetValue(exposure_i);
        //camera->Close();
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool CameraBasler::setAutoExposure(bool enable)
{
    try
    {
        std::string enable_str = "Off";
        if (enable){
            enable_str = "Continuous";
        }
        camera->Open();
        Pylon::CEnumParameter(camera->GetNodeMap(), "ExposureAuto").FromString(enable_str.c_str());
        //Pylon::CStringParameter(camera->GetNodeMap(), "ExposureAuto").SetValue(enable_str.c_str());
        //camera->Close();
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool CameraBasler::setGain(double gain)
{
    try
    {
        int gain_i = gain;
        camera->Open();
        Pylon::CIntegerParameter(camera->GetNodeMap(), "GainRaw").SetValue(gain_i);
        //camera->Close();
        return true;
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        return false;
    }
}

bool CameraBasler::capture(void)
{
    bool res = false;
    try
    {
        if (camera){
            if (camera->IsGrabbing())
            {
                Pylon::CGrabResultPtr ptrGrabResult;
                camera->RetrieveResult(5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException);
                if (ptrGrabResult == NULL){
                    qDebug() << "Camera grab pointer is null";
                    res = false;
                } else {
                    if (ptrGrabResult->GrabSucceeded())
                    {
                        int frameCols = ptrGrabResult->GetWidth();
                        int frameRows = ptrGrabResult->GetHeight();

                        if (frameCols == 0 || frameRows == 0){
                            qDebug() << "Image buffer size is incorrect";
                            qDebug() << "cols: " << frameCols << " rows: " << frameRows;
                            res = false;
                        } else {
                            Pylon::CPylonImage pylonImage;
                            formatConverter.Convert(pylonImage, ptrGrabResult);
                            //cv::Mat image_temp = cv::Mat(frameRows, frameCols, CV_8UC1, (uchar *) pylonImage.GetBuffer());
                            cv::Mat image_temp = cv::Mat(frameRows, frameCols, CV_8UC1, static_cast<uchar *>(pylonImage.GetBuffer()));
                            image_temp.copyTo(image);
                            if (image.cols == 0 || image.rows == 0){
                                qDebug() << "Image result buffer size is incorrect";
                                qDebug() << "cols: " << image.cols << " rows: " << image.rows;
                                res = false;
                            } else {
                                emit captured();
                                res = true;
                            }
                            pylonImage.Release();
                        }
                    }
                    else
                    {
                        qDebug() << "Failed to grab image.";
                        qDebug() << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription();
                        std::cerr << "Failed to grab image." << std::endl;
                        std::cerr << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
                        res = false;
                    }
                    ptrGrabResult.Release();
                }
            }
            else
            {
                qDebug() << "Camera isn't grabbing images.";
                std::cerr << "Camera isn't grabbing images." << std::endl;
                res = false;
            }
        } else {
            qDebug() << "Camera is null";
            std::cerr << "Camera is null" << std::endl;
            res = false;
        }
    }
    catch (const Pylon::GenericException &e)
    {
        // Error handling.
        qDebug() << "An exception occurred." << e.GetDescription();
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        res = false;
    }
    return res;
}

cv::Mat *CameraBasler::getImage(void) {
    if (image.cols == 0 || image.rows == 0){
        qDebug() << "Global image result buffer size is incorrect";
        qDebug() << "cols: " << image.cols << " rows: " << image.rows;
    }
    return &image;
}

CameraBasler::~CameraBasler(void)
{
    camera->StopGrabbing();
    camera->Close();
    // Releases all pylon resources.
    Pylon::PylonTerminate();
}