#include <VisionProcessor.h>
#include <QSettings>

extern QString g_strRootDir;

void VisionProcessor::getCameraObj(Camera* camera_obj)
{
    m_objCamera = camera_obj;
}

void VisionProcessor::setDetectTarget(uint16_t target)
{
    m_Control.detect_target = target;
}

void VisionProcessor::setEnemyColor(uint16_t color)
{
    m_Control.enemy_color = color;
}

void VisionProcessor::setShootSpeed(uint8_t speed)
{
    m_Control.shoot_speed = speed;
}

void VisionProcessor::setShowImage(uint16_t index)
{
    m_nShowIndex = index;
}

Mat* VisionProcessor::getShowImage()
{
    QMutexLocker locker(&m_objImageMutex);
    m_bShowFalse = false;
    return m_pShowImage;
}

bool VisionProcessor::showIsUpdate()
{
    QMutexLocker locker(&m_objImageMutex);
    return m_bShowFalse;
}

bool VisionProcessor::IsInitSucceed()
{
    if (!m_bIsCalibrate)
    {
        qDebug("Camera not calibrated!");
        return false;
    }

//    if (!m_bIsLoadDetectEngine)
//    {
//        qDebug("Not Load Windmill Engine.");
//        return false;
//    }

//    if (!m_bIsLoadWindmillEngine)
//    {
//        qDebug("Not Load Windmill Engine.");
//        return false;
//    }
    return true;
}

void VisionProcessor::setRedThreshold(uint8_t thres)
{
    m_nRedThreshold = thres;
}

void VisionProcessor::setBlueThreshold(uint8_t thres)
{
    m_nBlueThreshold = thres;
}

double VisionProcessor::getFps()
{
    m_objFps.UpdateFps();
    double dFrameRate = m_objFps.GetFps();
    return dFrameRate;
}

uint32_t VisionProcessor::getFrameCount()
{
    return m_nFrameCount;
}

VisionProcessor::VisionProcessor() :
    m_bRun(false),
    m_objDetect(nullptr),
    m_objWindmill(nullptr),
    m_nShowIndex(ShowSource),
    m_pShowImage(nullptr),
    m_objImageMutex(QMutex::Recursive),
    m_pSolver(nullptr)
{
    m_pArmorDetector = new ArmorDetector();

    ReadSettings();



    m_Control.detect_target = AllRobot;
    m_Control.enemy_color = Red;
}

VisionProcessor::~VisionProcessor()
{
    if (m_bRun)
    {
        m_bRun = false;
//        this->quit();
//        this->wait();
    }

    RELEASE_ALLOC_MEM(m_pShowImage);
    RELEASE_ALLOC_MEM(m_objDetect);
//    RELEASE_ALLOC_MEM(m_objWindmill);
    RELEASE_ALLOC_MEM(m_pArmorDetector);
    RELEASE_ALLOC_MEM(m_pSolver)
}

void VisionProcessor::ReadSettings()
{
    m_bIsCalibrate = false;
    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("Vision");
    m_nFlip = set->value("Flip", 0).toUInt();
    QStringList matrixT = set->value("CameraMatrix").toStringList();
    QStringList coeffsT = set->value("DistCoeffs").toStringList();
    QString strDetectLib = set->value("DetectLib").toString();
    QString strDetectEngineFile = set->value("DetectEngineFile", g_strRootDir + "/detect.engine").toString();
    QString strWindmillLib = set->value("WindmillLib").toString();
    QString strWindmillEngineFile = set->value("WindmillEngineFile", g_strRootDir + "/windmail.engine").toString();
    m_nRedThreshold = set->value("RedThreshold", 100).toUInt();
    m_nBlueThreshold = set->value("BlueThreshold", 100).toUInt();
    set->endGroup();

//    m_objDetect = new DetectRobot(strDetectLib, Yolo::DETECT_INPUT_H, Yolo::DETECT_INPUT_W,
//                                  Yolo::DETECT_CLASS_NUM, Yolo::DETECT_MAX_OUTPUT_BBOX_COUNT);
//    if (!m_objDetect->initLoadEngine(strDetectEngineFile.toStdString()))
//    {
//        qDebug("Load Detect Engine failed.");
//        m_bIsLoadDetectEngine = false;
//    }PreProcess
//    else
//    {|| 180 - angle_diff < T_ANGLE_THRE180
//        m_bIsLoadDetectEngine = true;
//    }

//    m_objWindmill = new DetectRobot(strWindmillLib, Yolo::WINDMILL_INPUT_H, Yolo::WINDMILL_INPUT_W,
//                                    Yolo::WINDMILL_CLASS_NUM, Yolo::WINDMILL_MAX_OUTPUT_BBOX_COUNT);
//    if (!m_objWindmill->initLoadEngine(strWindmillEngineFile.toStdString()))
//    {
//        qDebug("Load Windmill Engine failed.");
//        m_bIsLoadWindmillEngine = false;
//    }
//    else
//    {
//        m_bIsLoadWindmillEngine = true;
//    }


    if (matrixT.size() == 0 || coeffsT.size() == 0)
    {
        qDebug("CameraMatrix or DistCoeffs read size is empty.");
        return;
    }

    RELEASE_ALLOC_MEM(m_pSolver)
    //赋值相机内参
    double dCameraMatrix[9];
    double dDistCoeffs[5];

    for (uint8_t i = 0; i < matrixT.size(); i++)
    {
        dCameraMatrix[i] = matrixT.at(i).toDouble();
    }
    Mat matCameraMatrix = Mat(3,3,CV_64FC1,dCameraMatrix);

    for (uint8_t i = 0; i < coeffsT.size(); i++)
    {
        dDistCoeffs[i] = coeffsT.at(i).toDouble();
    }
    Mat matDistCoeffs = Mat(5,1,CV_64FC1, dDistCoeffs);
    m_pSolver = new AngleSolver(matCameraMatrix, matDistCoeffs, SMALL_ARMOR_WIDTH, SMALL_ARMOR_HEIGHT);

    m_bIsCalibrate = true;

}

void VisionProcessor::FirstFrameInit(Mat frame)
{
    int64_t i64ImageRows = frame.rows;
    int64_t i64ImageCols = frame.cols;

    RELEASE_ALLOC_MEM(m_pShowImage);

    try
    {
        m_pShowImage = new cv::Mat(i64ImageRows, i64ImageCols, CV_8UC3);
        // Allocate three QImage buffer for deque acquisition
    }
    catch (std::bad_alloc& e)
    {
       qDebug("Error: Start Acquisition Failed : Allocate image resources failed! ");
       return;
    }
}

void VisionProcessor::UpdateShowImage(const Mat& image)
{
    QMutexLocker locker(&m_objImageMutex);
    m_bShowFalse = true;
    image.copyTo(*m_pShowImage);
}


void VisionProcessor::run()
{
    m_bRun = true;
    m_nFrameCount = 0;
    std::vector<DetectResult_t> vecDetectResults;
    std::vector<RotatedRect> vecLights;
    std::vector<ArmorBox> vecArmorBoxs;

    uint32_t loss_cnt = 0;
    while (m_bRun)
    {
        if (!m_objCamera->isOpen() && !m_objCamera->isCapture() && !IsInitSucceed())
            continue;

        Mat* frame = m_objCamera->getFrame(); //取出一帧数据
        if (frame == nullptr)
            continue;
        Mat src = (*frame).clone();

        //第一帧，进行一些参数初始化
        if (m_nFrameCount == 0)
            FirstFrameInit(src);

        if (m_nFlip == XFlip)
            cv::flip(src, src, 0);
        else if (m_nFlip == YFlip)
            cv::flip(src, src, 1);
        else if (m_nFlip == XYFlip)
            cv::flip(src, src, -1);


        lastArmorbox = targetArmorbox;
        targetArmorbox = ArmorBox();
        if (m_Control.detect_target == Windmill)
        {

        }
        else
        {
            m_pArmorDetector->enemy_color = m_Control.enemy_color;
            m_pArmorDetector->blue_thres = m_nBlueThreshold;
            m_pArmorDetector->red_thres = m_nRedThreshold;
            m_pArmorDetector->getTargetAera(src, targetArmorbox);
//            qDebug("color: %d",m_Control.enemy_color);


            if (m_pArmorDetector->is_lost)
            {
//                eVisionState = VISION_TRACK_LOSS;
//                targetArmorbox.distance = MAX_DISTANCE;
//                targetArmorbox.pitch = 0;
//                targetArmorbox.yaw = 0;
            }
            else
            {
                double pitch, yaw, distance;
                if (targetArmorbox.type == SmallArmor)
                {
                    m_pSolver->setTargetSize(SMALL_ARMOR_WIDTH, SMALL_ARMOR_HEIGHT);

                }
                else if (targetArmorbox.type == BigArmor)
                {
                    m_pSolver->setTargetSize(BIG_ARMOR_WIDTH, BIG_ARMOR_HEIGHT);
                }
                else
                {
                    m_pSolver->setTargetSize(SMALL_ARMOR_WIDTH, SMALL_ARMOR_HEIGHT);
                }
                m_pSolver->getAngle(targetArmorbox.points, pitch, yaw, distance);

//                eVisionState = VISION_TRACK;
                targetArmorbox.distance = distance;
                targetArmorbox.pitch = pitch;
                targetArmorbox.yaw = yaw;

            }

            if (!m_pArmorDetector->is_lost)
            {
                eVisionState = VISION_TRACK;
                loss_cnt = 0;
            }
            else
            {
                if (loss_cnt <= 50)
                {
                    eVisionState = VISION_TRACK;
                    targetArmorbox.yaw = lastArmorbox.yaw * 0.95;
                    targetArmorbox.pitch = lastArmorbox.pitch * 0.95;
                    targetArmorbox.distance = lastArmorbox.distance;
//                    targetArmorbox = lastArmorbox;
                }
                else
                    eVisionState = VISION_TRACK_LOSS;
                loss_cnt++;
            }

            if (m_nShowIndex == ShowThreshold)
            {
                UpdateShowImage(m_pArmorDetector->getShowImage(ArmorDetector::SHOW_BINARY));
            }
            else if (m_nShowIndex == ShowDetect1)
            {
                UpdateShowImage(m_pArmorDetector->getShowImage(ArmorDetector::FIRST_RESULT));
            }
            else if (m_nShowIndex == ShowDetect2)
            {
                UpdateShowImage(m_pArmorDetector->getShowImage(ArmorDetector::SECOND_RESULT));
            }
            else
            {
                m_pArmorDetector->getShowImage(ArmorDetector::NOT_SHOW);
            }

        }


        if (m_nShowIndex ==ShowSource)
        {
            if (targetArmorbox.distance != MAX_DISTANCE)
            {
                cv::Scalar color(0, 255, 0);
                if (m_Control.enemy_color == Red)
                    color = cv::Scalar(255, 255, 0);
                else if (m_Control.enemy_color == Blue)
                    color = cv::Scalar(0, 255, 255);
                cv::circle(src, targetArmorbox.rect.center, 5, color, 3);
                qDebug("%f, %f, %f", targetArmorbox.distance, targetArmorbox.pitch, targetArmorbox.yaw);
            }
            UpdateShowImage(src);
        }

        m_Result.state = eVisionState;
        m_Result.distance = targetArmorbox.distance;
        m_Result.pitch = targetArmorbox.pitch;
        m_Result.yaw = targetArmorbox.yaw;
        m_Result.robot_num = 1;
        m_Result.fire = 0;
        m_Result.update = 1;
        emit sigSendVisionResult(m_Result);

        m_nFrameCount++;
        m_objFps.IncreaseFrameNum();
    }

    m_objFps.Reset();
    vecLights.clear();
    vecArmorBoxs.clear();

}
