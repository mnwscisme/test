#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QSettings>
#include <QCheckBox>

extern QString g_strRootDir;
const QString dateformat = "yyyyMMdd-hhmmss-zzz";
#define VIDEO_DIR "Video"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_uiCalibration(nullptr),
    m_bSaveImage(false),
    m_strSaveFolder("")
{
    ui->setupUi(this);
//    QFont font = this->font();
//    font.setPointSize(10);
//    this->setFont(font);

    ClearUI();
    SetUiEnabled(false);
    InitDialogs();
    ReadSettings();
    ComboBox_TargetInit();
    ComboBox_ColorInit();

    m_uTimerId_1S = startTimer(1000);
    m_uTimerId_Camera = startTimer(CAMERA_SHOW_TIMER);
}

MainWindow::~MainWindow()
{
    DestroyDialogs();
    delete ui;
}

void MainWindow::getCameraObj(Camera* camera)
{
    m_objCamera = camera;
}

void MainWindow::getVisionObj(VisionProcessor* vision)
{
    m_objVision = vision;
}

void MainWindow::timerEvent(QTimerEvent* event)
{
    static bool last_state = false;
    if (event->timerId() == m_uTimerId_1S)
    {
        if (m_objCamera->isOpen() && last_state == false )
        {
            SetUiEnabled(true);
            ui->VendorName->setText(QString("VendorName: %1").arg(m_objCamera->getVendorName()));
            ui->ModelName->setText(QString("ModelName: %1").arg(m_objCamera->getModelName()));
        }
        else if (!m_objCamera->isOpen() && last_state )
        {
            ClearUI();
            SetUiEnabled(false);
        }

        ShowFrameRate();
        last_state = m_objCamera->isOpen();
    }
    else if (event->timerId() == m_uTimerId_Camera)
    {
        ShowCameraFrame();
    }
}

void MainWindow::ClearUI()
{
    // Show device information on Info label
    ui->VendorName->setText(QString("<No Device Opened>"));
    ui->ModelName->clear();


    // Reset acquisition frame rate to 0
    ui->AcqFrameRateLabel->setText(QString("Acq. Frame NUM: %1 FPS: %2")
                                   .arg(0)
                                   .arg(0.0, 0, 'f', 1));
    // Reset manage frame rate
    ui->ManageFrameRateLabel->setText(QString("Manage. Frame NUM: %1 FPS: %2")
                                      .arg(0)
                                      .arg(0.0, 0, 'f', 1));
    // Reset show frame rate
    ui->ShowFrameRateLabel->setText(QString("Show.FPS: %1").arg(0.0, 0, 'f', 1));

    // Clear show image label
    ui->ImageLabel->clear();
}

void MainWindow::SetUiEnabled(bool en)
{
    ui->SourceImage->setEnabled(en);
    ui->OpencvImage->setEnabled(en);
    ui->DetectImage->setEnabled(en);
    ui->Calibration->setEnabled(en);
    ui->ComboBox_Target->setEnabled(en);
    ui->ComboBox_Color->setEnabled(en);
    ui->Recording->setEnabled(en);
    ui->RedThreshold->setEnabled(en);
    ui->BlueThreshold->setEnabled(en);
    ui->ThresholdImage->setEnabled(en);

}

void MainWindow::ComboBox_TargetInit()
{
    ui->ComboBox_Target->addItem("AllRobot", AllRobot);
    ui->ComboBox_Target->addItem("Windmill", Windmill);
//    ui->ComboBox_Target->addItem("Hero", Hero);
//    ui->ComboBox_Target->addItem("Engineer", Engineer);
//    ui->ComboBox_Target->addItem("Infantry", Infantry);
//    ui->ComboBox_Target->addItem("Sentey", Sentey);
}

void MainWindow::ComboBox_ColorInit()
{
//    ui->ComboBox_Color->addItem("AllColor", AllColor);
    ui->ComboBox_Color->addItem("Red", Red);
    ui->ComboBox_Color->addItem("Blue", Blue);
}

void MainWindow::InitDialogs()
{
    // Release dialogs
    DestroyDialogs();

    try
    {
        // Instantiation dialogs
        m_uiCalibration         = new CalibrationDialog;
    }
    catch (std::bad_alloc &e)
    {
        qDebug("Cannot allocate memory, please exit this app!");
        DestroyDialogs();
        return;
    }

    connect(m_uiCalibration, &CalibrationDialog::sigSaveImage, this, [=](){m_bSaveImage = true;});
}

void MainWindow::CloseDialogs()
{
    m_uiCalibration->close();
}

void MainWindow::DestroyDialogs()
{
    RELEASE_ALLOC_MEM(m_uiCalibration);
}

double MainWindow::GetImageShowFps()
{
    double dImgShowFrameRate = 0.0;

    m_objFps.UpdateFps();
    dImgShowFrameRate = m_objFps.GetFps();

    return dImgShowFrameRate;
}

void MainWindow::ShowCameraFrame()
{
    static bool show_update = false;
#ifndef OPENCV4_DLL
    qDebug("Not detected Opencv Lib!!!");
    return;
#endif
    if (ui->ShowClose->isChecked())
        return;

    if (m_objCamera->isOpen() && m_objCamera->isCapture())
    {
        cv::Mat* frame = nullptr;
        if (ui->SourceImage->isChecked())
        {
            frame = m_objCamera->getFrame();
            show_update = true;
        }
        else if (ui->OpencvImage->isChecked() ||
                 ui->DetectImage->isChecked() ||
                 ui->ThresholdImage->isChecked()||
                 ui->DetectImage_2->isChecked())
        {
            if (m_objVision->showIsUpdate())
            {
                frame = m_objVision->getShowImage();
                show_update = true;
            }
        }
        else
        {
            frame = m_objCamera->getFrame();
            show_update = true;
        }
        if (frame == nullptr)
            return;

        QImage objImgShow;
        if (frame->type() == CV_8UC1 || frame->type() == CV_8U)
            objImgShow = QImage(frame->data, frame->cols, frame->rows, QImage::Format_Grayscale8 );
        else
            objImgShow = QImage(frame->data, frame->cols, frame->rows, QImage::Format_RGB888 );

        RecordingVideo();
        SaveImage(objImgShow, m_strSaveFolder);

        // Display the image
        QImage objImgScaled = objImgShow.scaled(ui->ImageLabel->width(), ui->ImageLabel->height(),
                                                Qt::IgnoreAspectRatio, Qt::FastTransformation);
        ui->ImageLabel->setPixmap(QPixmap::fromImage(objImgScaled));

        if (show_update)
            m_objFps.IncreaseFrameNum();


        show_update = false;
    }

}

void MainWindow::ShowFrameRate()
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
    {
        m_objFps.Reset();
        return;
    }

    double dAcqFrameRate = m_objCamera->getFps();
    double dManageFrameRate = m_objVision->getFps();
    double dImgShowFrameRate = GetImageShowFps();

    // Reset acquisition frame rate to 0
    ui->AcqFrameRateLabel->setText(QString("Acq. Frame NUM: %1  FPS: %2")
                                   .arg(m_objCamera->getFrameCount())
                                   .arg(dAcqFrameRate, 0, 'f', 1));
    // Reset manage frame rate
    ui->ManageFrameRateLabel->setText(QString("Manage. Frame NUM: %1 FPS: %2")
                                      .arg(m_objVision->getFrameCount())
                                      .arg(dManageFrameRate, 0, 'f', 1));
    // Reset show frame rate
    ui->ShowFrameRateLabel->setText(QString("Show.FPS: %1").arg(dImgShowFrameRate, 0, 'f', 1));
}

void MainWindow::SaveImage(const QImage& image, QString folder)
{
    if (m_bSaveImage)
    {
        if (!folder.isEmpty())
        {
            QDir dir(g_strRootDir);
            if (!dir.exists(folder))
            {
                if(dir.mkpath(folder))
                {
                    qDebug("Create folder: %s", qPrintable(folder));
                }
            }
        }

        QString path = g_strRootDir + "/" + folder + "/" + (QDateTime::currentDateTime().toString(dateformat)+".png");
        if (!image.save(path))
            qDebug("Save image faild!");
        else
            qDebug("Save image succeed!");

        m_bSaveImage = false;
    }
}

void MainWindow::RecordingVideo()
{
    if (!m_uiCalibration->isHidden())
        ui->Recording->setCheckState(Qt::Unchecked);

    if (!ui->OpencvImage->isChecked() && ui->Recording->isChecked())
        ui->OpencvImage->setChecked(true);
    if (ui->Recording->isChecked())
    {
        m_strSaveFolder = VIDEO_DIR;
        m_bSaveImage = true;
    }
}

void MainWindow::ReadSettings()
{
    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("UI");
    bool rec = set->value("Recording", false).toBool();
    if (rec)
        ui->Recording->setCheckState(Qt::Checked);
    set->endGroup();

    set->beginGroup("Vision");
    int red = set->value("RedThreshold", 100).toUInt();
    ui->RedValue->setText(QString("%1").arg(red));
    ui->RedThreshold->setValue(red);

    int blue = set->value("BlueThreshold", 100).toUInt();
    ui->BlueValue->setText(QString("%1").arg(blue));
    ui->BlueThreshold->setValue(blue);
    set->endGroup();
}

void MainWindow::on_Calibration_clicked()
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;

    // Close all dialogs
    CloseDialogs();
    ui->OpencvImage->setChecked(true);

    // Get dialog initial param
    m_uiCalibration->setCameraParam(m_objCamera);

    // Make active window can be modify by click
    m_uiCalibration->setWindowModality(Qt::WindowModal);

    // Display dialog and raise it to the front
    m_uiCalibration->show();
    m_uiCalibration->raise();
    m_uiCalibration->activateWindow();

    // Make dialog size unchangable
    m_uiCalibration->setFixedSize(m_uiCalibration->width(), m_uiCalibration->height());

    m_strSaveFolder = CALIBRATION_DIR;
}

void MainWindow::on_SourceImage_clicked()
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
}

void MainWindow::on_OpencvImage_clicked()
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setShowImage(ShowSource);
}

void MainWindow::on_DetectImage_clicked()
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setShowImage(ShowDetect1);
}

void MainWindow::on_Recording_clicked(bool checked)
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("UI");
    set->setValue("Recording", checked);
    set->endGroup();
}

void MainWindow::on_ComboBox_Target_currentIndexChanged(int index)
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setDetectTarget(ui->ComboBox_Target->itemData(index).toInt());
}

void MainWindow::on_ComboBox_Color_currentIndexChanged(int index)
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setEnemyColor(ui->ComboBox_Color->itemData(index).toInt());
}

void MainWindow::on_RedThreshold_sliderMoved(int position)
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setRedThreshold(position);
    ui->RedValue->setText(QString("%1").arg(position));

    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("Vision");
    set->setValue("RedThreshold", position);
    set->endGroup();
}

void MainWindow::on_BlueThreshold_sliderMoved(int position)
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setBlueThreshold(position);
    ui->BlueValue->setText(QString("%1").arg(position));

    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("Vision");
    set->setValue("BlueThreshold", position);
    set->endGroup();
}

void MainWindow::on_ThresholdImage_clicked()
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setShowImage(ShowThreshold);
}

void MainWindow::on_DetectImage_2_clicked()
{
    if (!m_objCamera->isOpen() || !m_objCamera->isCapture())
        return;
    m_objVision->setShowImage(ShowDetect2);
}
