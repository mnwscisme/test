#ifndef VISION_PROC_H
#define VISION_PROC_H

#include <QThread>
#include <QMutex>
#include <deque>
#include <opencv2/opencv.hpp>
#include "Singleton.h"
#include "Camera.h"
#include "fps.h"
#include "DetectRobot.h"
#include "Protocol.h"
#include "ArmorDetector.h"
#include "armorbox.h"

using namespace cv;

enum VisionFlip
{
    NotFlip = 0,
    XFlip = 1,
    YFlip = 2,
    XYFlip = XFlip|YFlip,
};

enum VisionShow
{
    ShowSource = 0,
    ShowDetect1,
    ShowDetect2,
    ShowThreshold
};

class VisionProcessor : public QThread
{
    Q_OBJECT
    DECLARE_SINGLETON(VisionProcessor);

public:
    enum BinaryMode
    {
        RGB = 1,
        HSV = 2,
        RGB2G = 3,
        OTSU = 4,
        GRAY = 5,
        YCrCb = 6,
        LUV = 7
    };

    ~VisionProcessor();
    void getCameraObj(Camera* camera_obj);

    void setDetectTarget(uint16_t target);
    void setEnemyColor(uint16_t color);
    void setShootSpeed(uint8_t speed);

    void setShowImage(uint16_t index);
    Mat* getShowImage();      //取出一帧图像
    bool showIsUpdate();
    bool IsInitSucceed();
    void setRedThreshold(uint8_t thres);
    void setBlueThreshold(uint8_t thres);

    double getFps();
    uint32_t getFrameCount();

signals:
    void sigSendVisionResult(VisionResult_t res);

protected:
    void run();

private:
    VisionProcessor();
    void ReadSettings();
    void FirstFrameInit(Mat frame);
    void UpdateShowImage(const Mat& image);

    bool                    m_bRun;

    Camera*                 m_objCamera;

    /*--------图像运行使用参数----------*/
    uint8_t                 m_nFlip;
    bool                    m_bIsCalibrate;
    bool                    m_bIsLoadDetectEngine;
    bool                    m_bIsLoadWindmillEngine;
    // deep learning
    DetectRobot*            m_objDetect;
    DetectRobot*            m_objWindmill;

    uint8_t m_nRedThreshold;
    uint8_t m_nBlueThreshold;

    ArmorDetector* m_pArmorDetector;
    AngleSolver* m_pSolver;

    VisionState_e eVisionState = VISION_TRACK_LOSS;
    VisionState_e eLastVisionState = VISION_TRACK_LOSS;
    ArmorBox targetArmorbox, lastArmorbox;
    /*--------图像运行使用参数----------*/

    VisionControl_t         m_Control;
    VisionResult_t          m_Result;


    //显示进程使用
    uint16_t                m_nShowIndex;
    bool                    m_bShowFalse;
    Mat*                    m_pShowImage;
    QMutex                  m_objImageMutex;       ///< Mutex for deque

    //帧率统计与计算
    CFps                    m_objFps;                   ///< Fps calulate object
    uint32_t                m_nFrameCount;              ///< frame count
};

#endif // VISION_PROC_H
