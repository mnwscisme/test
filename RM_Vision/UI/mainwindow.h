#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "Camera.h"
#include "VisionProcessor.h"
#include "calibrationdialog.h"

#define CAMERA_SHOW_TIMER (33)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void getCameraObj(Camera* camera);
    void getVisionObj(VisionProcessor* vision);

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void on_Calibration_clicked();
    void on_SourceImage_clicked();
    void on_OpencvImage_clicked();
    void on_DetectImage_clicked();
    void on_Recording_clicked(bool checked);
    void on_ComboBox_Target_currentIndexChanged(int index);
    void on_ComboBox_Color_currentIndexChanged(int index);
    void on_RedThreshold_sliderMoved(int position);
    void on_BlueThreshold_sliderMoved(int position);
    void on_ThresholdImage_clicked();

    void on_DetectImage_2_clicked();

private:
    /// Clear Mainwindow items
    void ClearUI();
    void SetUiEnabled(bool en);
    void ComboBox_TargetInit();
    void ComboBox_ColorInit();
    void InitDialogs();
    void CloseDialogs();
    void DestroyDialogs();

    double GetImageShowFps();
    void ShowCameraFrame();
    void ShowFrameRate();

    void SaveImage(const QImage& image, QString folder = "");
    void RecordingVideo();
    void ReadSettings();

    Ui::MainWindow*     ui;
    CalibrationDialog*  m_uiCalibration;

    Camera*             m_objCamera;
    VisionProcessor*    m_objVision;

    qint16              m_uTimerId_1S;
    qint16              m_uTimerId_Camera;

    bool                m_bSaveImage;              ///< Flag : Save one image when it is true
    QString             m_strSaveFolder;
    QImage              m_objImageForSave;         ///< For image saving

    CFps                m_objFps;                  ///< Calculated image display fps
};

#endif // MAINWINDOW_H
