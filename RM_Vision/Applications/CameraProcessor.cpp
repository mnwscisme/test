#include "CameraProcessor.h"

#include <QSettings>

extern QString g_strRootDir;

void CameraProcessor::timerEvent(QTimerEvent* event)
{
    if (m_objCamera != nullptr)
    {
        CameraConnect();
    }
}

CameraProcessor::CameraProcessor()
{
    readSettings();
    m_objCamera = new Camera();
    CameraConnect();
    startTimer(1000);
}

void CameraProcessor::CameraConnect()
{
    if (!m_objCamera->isOpen())
    {
        qDebug("Camera Try to connect!!!");
        GX_DEVICE_BASE_INFO info;
        uint32_t num;
        m_objCamera->updateDeviceList(&info, &num);
        if (num != 0)
        {
            m_objCamera->open(1);
            m_objCamera->setWhiteBalance(m_dBalanceRed, m_dBalanceGreen, m_dBalanceBlue);
            m_objCamera->setExposureTime(m_dExposureTime);
            m_objCamera->setGain(m_dGain);
            m_objCamera->startCapture();
        }
    }
}

void CameraProcessor::readSettings()
{
    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("Camera");
    m_dBalanceRed = set->value("BalanceRed", 1.0).toDouble();
    m_dBalanceGreen = set->value("BalanceGreen", 1.0).toDouble();
    m_dBalanceBlue = set->value("BalanceBlue", 1.0).toDouble();
    m_dExposureTime = set->value("ExposureTime", 10000.0).toDouble();
    m_dGain = set->value("Gain", 5.0).toDouble();
    set->endGroup();
}



