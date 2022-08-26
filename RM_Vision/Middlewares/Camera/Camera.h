#ifndef CAMERA_H
#define CAMERA_H

#include <QThread>

#include <QMutex>
#include <deque>

#include "GalaxySDK/linux/inc/GxIAPI.h"
#include "GalaxySDK/linux/inc/DxImageProc.h"
#include "Common.h"
#include "fps.h"

#define ENUMERATE_TIME_OUT      200

/// Show Error Message
#define CAMERA_SHOW_MESSAGA(emStatus) qDebug("Error message: %d", emStatus);

typedef GX_FRAME_DATA CAMERA_FRAME_BUFFER;

#ifdef OPENCV4_DLL
#include <opencv2/opencv.hpp>
typedef cv::Mat CameraImage;
#else
#include <QImage>
typedef QImage CameraImage;
#endif

class Camera : public QThread
{
    Q_OBJECT
public:
    explicit Camera(QObject* parent = nullptr);
    ~Camera();

    GX_STATUS open(uint32_t index);
    GX_STATUS open(GX_OPEN_MODE_CMD open_mode, char* content, GX_ACCESS_MODE_CMD access_mode = GX_ACCESS_EXCLUSIVE);
    GX_STATUS close();

    GX_STATUS deviceInit();
    GX_STATUS updateDeviceList(GX_DEVICE_BASE_INFO* base_info, uint32_t* device_num);
    void startCapture();
    void stopCapture();
    GX_DEV_HANDLE getDeviceHandle();

    GX_STATUS setWhiteBalance(double red_ratio, double green_ratio, double blue_ratio);
    GX_STATUS setExposureTime(double exposure_time);
    GX_STATUS setGain(double gain);
    QString getVendorName();
    QString getModelName();
    QString getSerialNumber();
    QString getDeviceVersion();

    bool isOpen();
    bool isCapture();

    CameraImage* getFrame();      //取出一帧图像
    double getFps();
    uint32_t getFrameCount();
signals:

protected:
    void run();

private:
    void EmptyBufferDeque_PushBack(CameraImage* image);


    /// Process Raw image to RGB image and do image improvment
    bool ImageProcess(const CAMERA_FRAME_BUFFER* frame_buffer, unsigned char* pixel_data);

    void setAcquisitionBufferNum();

    bool                    m_bOpen;                  ///< 相机打开标志
    bool                    m_bCapture;               ///< 相机开始采集标志

    GX_DEV_HANDLE           m_hDevice;                  ///< 设备句柄
    int64_t                 m_i64PayLoadSize;           ///< 设备输出原始图像大小
    int64_t                 m_i64ImageWidth;            ///< 相机输出图像宽度
    int64_t                 m_i64ImageHeight;           ///< 相机输出图像高度
    int64_t                 m_i64ColorFilter;           ///< 彩色相机的Bayer格式
    bool                    m_bColorFilter;             ///< 判断相机是否支持Bayer格式


    CAMERA_FRAME_BUFFER*    m_pFrameBuffer;             ///< Array of PGX_FRAME_BUFFER
    uint64_t                m_ui64BufferNum;            ///< 采集时帧数
    unsigned char*          m_pRaw8Image;               ///< Intermediate variables between DxRaw16toRaw8 and DxRaw8toRGB24
    CameraImage*            m_pCameraImage;

    QMutex                  m_objParamMutex;            ///< Mutex for cross thread parameters
    QMutex                  m_objImageMutex;            ///< Mutex for deque

    CFps                    m_objFps;                   ///< Fps calulate object
    uint32_t                m_nFrameCount;              ///< frame count
};

#endif // CAMERA_H
