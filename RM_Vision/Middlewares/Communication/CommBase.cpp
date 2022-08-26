#include "CommBase.h"

ReceiveBase::ReceiveBase(std::function<void (QByteArray&)> unpack_func, uint16_t period_time, QObject* parent) : QObject(parent)
{
    pUnpackFunc = unpack_func;
    startTimer(period_time);
}

void ReceiveBase::receiveData(const QByteArray& data)
{
    m_byteReceiveBuffer.append(data);
}

void ReceiveBase::setUnpackFunc(std::function<void (QByteArray&)> unpack_func)
{
    pUnpackFunc = unpack_func;
}

void ReceiveBase::timerEvent(QTimerEvent* event)
{
    receiveDataHandler();
}

void ReceiveBase::receiveDataHandler()
{
    if (pUnpackFunc != nullptr)
    {
        pUnpackFunc(m_byteReceiveBuffer);
    }
    else
    {
        qDebug("pUnpackFunc == nullptr");
    }
}

TransmitBase::TransmitBase(uint16_t period_time, QObject* parent) : QObject(parent)
{
    startTimer(period_time);
}

void TransmitBase::transmitDataBase(const QByteArray& data)
{
    m_byteTransmitBuffer = data;
}

void TransmitBase::timerEvent(QTimerEvent* event)
{
    transmitDataHandler();
}

void TransmitBase::transmitDataHandler()
{
    if (!m_byteTransmitBuffer.isEmpty())
    {
        emit readyTransmit(m_byteTransmitBuffer);
//        m_byteTransmitBuffer.clear();
    }
}
