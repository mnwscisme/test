#include "calibrationdialog.h"
#include "ui_calibrationdialog.h"
#include <QDir>
#include <vector>
#include <QSettings>

extern QString g_strRootDir;

CalibrationDialog::CalibrationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalibrationDialog),
    m_dCameraMatrix(nullptr),
    m_nMatrixSize(0),
    m_dDistCoeffs(nullptr),
    m_nCoeffsSize(0)
{
    ui->setupUi(this);
    ReadSettings();
}

CalibrationDialog::~CalibrationDialog()
{
    delete ui;
}

void CalibrationDialog::setCameraParam(Camera* camera)
{
    camera->setWhiteBalance(m_dBalanceRed, m_dBalanceGreen, m_dBalanceBlue);
    camera->setExposureTime(m_dExposureTime);
    camera->setGain(m_dGain);
}


void CalibrationDialog::on_SaveImage_clicked()
{
    emit sigSaveImage();
}

void CalibrationDialog::on_Cali_clicked()
{
    Calibration();
}

void CalibrationDialog::on_Save_clicked()
{
    WriteSettings();
}

void CalibrationDialog::Calibration()
{
    QDir cali_dir(g_strRootDir + "/" + CALIBRATION_DIR);
    if (!cali_dir.exists())
    {
        qDebug("Calibration dir not exists!");
        return;
    }

    int image_count = 0;  /* 图像数量 */
    Size image_size;      /* 图像的尺寸 */
    Size board_size = Size(m_nChessboardCols, m_nChessboardRwos);               /* 标定板上每行、列的角点数 */
    std::vector<Point2f> points_buf;            /* 缓存每幅图像上检测到的角点 */
    std::vector<std::vector<Point2f>> points_seq;  /* 保存检测到的所有角点 */
    Mat input;

    qDebug("------Start camera calibration.------");
    /* 读取每一幅图像，从中提取出角点，然后对角点进行亚像素精确化 */
    qDebug("------Extract the corners.------");
    //获取图片路径
    QFileInfoList fileInfoList = cali_dir.entryInfoList();
    foreach (QFileInfo fileInfo, fileInfoList)
    {
        if(fileInfo.isFile() )
        {
            QString ext = fileInfo.completeSuffix();
            if (ext == "bmp" || ext =="png" || ext =="jpeg")
            {
                qDebug("%s", qPrintable(fileInfo.absoluteFilePath()));
                input = imread(fileInfo.absoluteFilePath().toStdString());
                /* 读入第一张图片时获取图片大小 */
                if (image_count == 0)
                {
                    image_size.width = input.cols;
                    image_size.height = input.rows;
                    qDebug("image_size.width = %d, image_size.height = %d", image_size.width, image_size.height);
                }

                if (0 == findChessboardCorners(input, board_size, points_buf, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FILTER_QUADS))
                {
                    qDebug("can not find chessboard corners!");
                }
                else
                {
                    Mat view_gray;
                    cvtColor(input, view_gray, COLOR_RGB2GRAY);  // 转灰度图

                    /* 亚像素精确化
                     * points_buf 初始的角点坐标向量，同时作为亚像素坐标位置的输出
                     * Size(5,5) 搜索窗口大小
                     * (-1，-1)表示没有死区
                     * TermCriteria 角点的迭代过程的终止条件, 可以为迭代次数和角点精度两者的组合
                     */
                    cornerSubPix(view_gray, points_buf, Size(5,5), Size(-1,-1), TermCriteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.1));
                    points_seq.push_back(points_buf);  // 保存亚像素角点

                    /* 在图像上显示角点位置 */
                    drawChessboardCorners(view_gray, board_size, points_buf, false); // 用于在图片中标记角点
                    imshow("Camera Calibration", view_gray);       // 显示图片
                    waitKey(500); //暂停0.5S
                    image_count++;
                }
            }
        }
    }

    int CornerNum = board_size.width * board_size.height;  // 每张图片上总的角点数

    qDebug("------In the calibration------");
    //相机标定
    /*棋盘三维信息*/
    Size square_size = Size(m_nChessboardSquare, m_nChessboardSquare);         /* 实际测量得到的标定板上每个棋盘格的大小 */
    std::vector<std::vector<Point3f> > object_points;   /* 保存标定板上角点的三维坐标 */

    /*内外参数*/
    Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));  /* 摄像机内参数矩阵 */
    std::vector<int> point_counts;   // 每幅图像中角点的数量
    Mat distCoeffs=Mat(1, 5, CV_32FC1,Scalar::all(0));       /* 摄像机的5个畸变系数：k1,k2,p1,p2,k3 */
    std::vector<Mat> tvecsMat;      /* 每幅图像的旋转向量 */
    std::vector<Mat> rvecsMat;      /* 每幅图像的平移向量 */

    /* 初始化标定板上角点的三维坐标 */
    int i, j, t;
    for (t=0; t<image_count; t++) //第几张图片
    {
        std::vector<Point3f> tempPointSet;
        for (i=0; i<board_size.height; i++)
        {
            for (j=0; j<board_size.width; j++)
            {
                Point3f realPoint;
                /* 假设标定板放在世界坐标系中z=0的平面上 */
                realPoint.x = i * square_size.width;
                realPoint.y = j * square_size.height;
                realPoint.z = 0;
                tempPointSet.push_back(realPoint);
            }
        }
        object_points.push_back(tempPointSet);
    }

    /* 初始化每幅图像中的角点数量，假定每幅图像中都可以看到完整的标定板 */
    for (i=0; i<image_count; i++)
    {
        point_counts.push_back(board_size.width * board_size.height);
    }

    /* 开始标定
     * object_points 世界坐标系中的角点的三维坐标
     * points_seq 每一个内角点对应的图像坐标点
     * image_size 图像的像素尺寸大小
     * cameraMatrix 输出，内参矩阵
     * distCoeffs 输出，畸变系数
     * rvecsMat 输出，旋转向量
     * tvecsMat 输出，位移向量
     * 0 标定时所采用的算法
     */
    calibrateCamera(object_points, points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);
    qDebug("------Calibration completed------");
    //------------------------标定完成------------------------------------
    //-------------------对标定结果进行评价------------------------------
    double total_err = 0.0;         /* 所有图像的平均误差的总和 */
    double err = 0.0;               /* 每幅图像的平均误差 */
    std::vector<Point2f> image_points2;  /* 保存重新计算得到的投影点 */

    for (i=0;i<image_count;i++)//第几张图片
    {
        std::vector<Point3f> tempPointSet = object_points[i];

        /* 通过得到的摄像机内外参数，对空间的三维点进行重新投影计算，得到新的投影点 */
        projectPoints(tempPointSet, rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points2);

        /* 计算新的投影点和旧的投影点之间的误差*/
        std::vector<Point2f> tempImagePoint = points_seq[i];
        Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
        Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);

        for (int j = 0 ; j < tempImagePoint.size(); j++)
        {
            image_points2Mat.at<Vec2f>(0,j) = Vec2f(image_points2[j].x, image_points2[j].y);
            tempImagePointMat.at<Vec2f>(0,j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
        }
        err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
        total_err += err/= point_counts[i];
//        qDebug("%d image mean error pixel: %f", i+1, err);
    }
    qDebug("total mean error pixel: %f", total_err/image_count);
    //-------------------------评价完成---------------------------------------------
    //-----------------------保存定标结果-------------------------------------------
    RELEASE_ALLOC_ARR(m_dCameraMatrix);
    RELEASE_ALLOC_ARR(m_dDistCoeffs);

    m_nMatrixSize = cameraMatrix.cols * cameraMatrix.rows;
    m_dCameraMatrix = new double[m_nMatrixSize];
    memcpy(m_dCameraMatrix, cameraMatrix.data, sizeof(double)*m_nMatrixSize);
//    for (uint8_t i = 0; i < m_nMatrixSize; i++)
//        qDebug("%f", m_dCameraMatrix[i]);

    m_nCoeffsSize = distCoeffs.cols * distCoeffs.rows;
    m_dDistCoeffs = new double[m_nCoeffsSize];
    memcpy(m_dDistCoeffs, distCoeffs.data, sizeof(double)*m_nCoeffsSize);
//    for (uint8_t i = 0; i < m_nCoeffsSize; i++)
//        qDebug("%f", m_dDistCoeffs[i]);

    qDebug("------Camera Matrix------");
    std::cout << cameraMatrix << std::endl;
    qDebug("------Dist Coeffs------");
    std::cout << distCoeffs << std::endl;
    //--------------------标定结果保存结束-------------------------------
    //---------------------------------查看标定效果——利用标定结果对棋盘图进行矫正-----

    Mat mapx = Mat(image_size, CV_32FC1);
    Mat mapy = Mat(image_size, CV_32FC1);
    Mat R = Mat::eye(3, 3, CV_32F);
    initUndistortRectifyMap(cameraMatrix, distCoeffs, R, cameraMatrix, image_size, CV_32FC1, mapx, mapy);
    foreach (QFileInfo fileInfo, fileInfoList)
    {
        QString ext = fileInfo.completeSuffix();
        if (ext == "bmp" || ext =="png" || ext =="jpeg")
        {
            qDebug("%s", qPrintable(fileInfo.absoluteFilePath()));
            Mat imageSource = imread(fileInfo.absoluteFilePath().toStdString());
            Mat newimage = imageSource.clone();
            remap(imageSource, newimage, mapx, mapy, INTER_LINEAR);
            imshow("Camera Calibration", newimage);       // 显示图片
            waitKey(500); //暂停0.5S
        }
    }
}

void CalibrationDialog::ReadSettings()
{
    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("Calibration");
    m_dBalanceRed = set->value("BalanceRed", 1.0).toDouble();
    m_dBalanceGreen = set->value("BalanceGreen", 1.0).toDouble();
    m_dBalanceBlue = set->value("BalanceBlue", 1.0).toDouble();
    m_dExposureTime = set->value("ExposureTime", 10000.0).toDouble();
    m_dGain = set->value("Gain", 5.0).toDouble();
    m_nChessboardCols = set->value("ChessboardCols", 9).toUInt();
    m_nChessboardRwos = set->value("ChessboardRwos", 6).toUInt();
    m_nChessboardSquare = set->value("ChessboardSquare", 24).toUInt();
    set->endGroup();
}

void CalibrationDialog::WriteSettings()
{
    if (m_nMatrixSize == 0 && m_dDistCoeffs == 0)
        return;

    QStringList strCameraMatrix;
    for (uint8_t i = 0; i < m_nMatrixSize; i++)
        strCameraMatrix << QString::number(m_dCameraMatrix[i]);

    QStringList strDistCoeffs;
    for (uint8_t i = 0; i < m_nCoeffsSize; i++)
        strDistCoeffs << QString::number(m_dDistCoeffs[i]);

    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("Vision");
    set->setValue("CameraMatrix", strCameraMatrix);
    set->setValue("DistCoeffs", strDistCoeffs);
    set->endGroup();
    qDebug("The calibration parameters are saved");
}
