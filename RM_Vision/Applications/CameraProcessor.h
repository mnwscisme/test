#ifndef CAMERA_PROC_H
#define CAMERA_PROC_H

#include <QObject>
#include "Singleton.h"
#include "Camera.h"

class CameraProcessor : public QObject
{
    Q_OBJECT
    DECLARE_SINGLETON(CameraProcessor);
public:
    Camera* m_objCamera;

signals:

private:
    void timerEvent(QTimerEvent *event);
    CameraProcessor();
    void CameraConnect();
    void readSettings();

    double m_dBalanceRed;
    double m_dBalanceGreen;
    double m_dBalanceBlue;
    double m_dExposureTime;
    double m_dGain;
};

#endif // CAMERA_PROC_H
