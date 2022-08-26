#ifndef CALIBRATIO_DIALOG_H
#define CALIBRATIO_DIALOG_H

#include <QDialog>
#include "Camera.h"
#include <opencv2/opencv.hpp>

#define CALIBRATION_DIR "Calibration"

using namespace cv;

namespace Ui {
class CalibrationDialog;
}

class CalibrationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalibrationDialog(QWidget *parent = nullptr);
    ~CalibrationDialog();

    void setCameraParam(Camera* camera);
signals:
    void sigSaveImage();

private slots:
    void on_SaveImage_clicked();
    void on_Cali_clicked();
    void on_Save_clicked();

private:
    void Calibration();
    void ReadSettings();
    void WriteSettings();

    Ui::CalibrationDialog *ui;

    double* m_dCameraMatrix;
    uint8_t m_nMatrixSize;
    double* m_dDistCoeffs;
    uint8_t m_nCoeffsSize;

    double m_dBalanceRed;
    double m_dBalanceGreen;
    double m_dBalanceBlue;
    double m_dExposureTime;
    double m_dGain;;
    uint8_t m_nChessboardCols;
    uint8_t m_nChessboardRwos;
    uint8_t m_nChessboardSquare;
};

#endif // CALIBRATIO_DIALOG_H
