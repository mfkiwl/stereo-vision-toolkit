/*
* Copyright I3D Robotics Ltd, 2020
* Author: Ben Knight (bknight@i3drobotics.com)
*/

#ifndef STEREOCAMERAVIMBA_H
#define STEREOCAMERAVIMBA_H

#include <opencv2/opencv.hpp>
#include <abstractstereocamera.h>
#include <VimbaCPP/include/VimbaCPP.h>

#include "ApiController.h"

//!  Stereo vimba cameras
/*!
  Control of stereo pair of vimba cameras and generation of 3D
*/

class StereoCameraVimba  : public AbstractStereoCamera
{
Q_OBJECT

public:

    explicit StereoCameraVimba (AbstractStereoCamera::StereoCameraSerialInfo serial_info,
                                AbstractStereoCamera::StereoCameraSettings camera_settings,
                                QObject *parent = 0) :
                AbstractStereoCamera(serial_info, camera_settings, parent),
                apiController_(AVT::VmbAPI::Examples::ApiController()){
        apiControllerStatus_ = apiController_.StartUp();
    }

    static std::vector<AbstractStereoCamera::StereoCameraSerialInfo> listSystems();

    ~StereoCameraVimba(void);

public slots:
    // Implimentations of virtual functions from parent class
    bool openCamera();
    bool closeCamera();
    bool captureSingle();
    bool enableCapture(bool enable);
    bool setFPS(int fps);
    bool setExposure(double exposure);
    bool enableAutoExposure(bool enable);
    bool setPacketSize(int){return false;} //NA
    bool setPacketDelay(int){return false;} //NA
    bool enableTrigger(bool enable);
    bool enableAutoGain(bool enable);
    bool setGain(int gain);
    bool setBinning(int binning);

    void captureThreaded();

private:
    QFuture<void> future;

    AVT::VmbAPI::Examples::ApiController apiController_;
    AVT::VmbAPI::CameraPtr camera_l, camera_r;

    bool setupCameras(AbstractStereoCamera::StereoCameraSerialInfo CSI_cam_info,int iBinning, int iTrigger, int iFps);

    VmbErrorType apiControllerStatus_;

    void getImageSize(int &width, int &height, int &bitdepth);
};

#endif //STEREOCAMERAVIMBA_H
