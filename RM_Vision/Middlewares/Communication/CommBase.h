#ifndef COMM_BASE_H
#define COMM_BASE_H

#include <iostream>
#include <functional>
#include <QObject>
#include <QtGlobal>
#include <QByteArray>

class ReceiveBase : public QObject
{
    Q_OBJECT
public:
    ReceiveBase(std::function<void (QByteArray&)> unpack_func, uint16_t period_time = 10, QObject* parent = nullptr);
    void receiveData(const QByteArray& data);
    void setUnpackFunc(std::function<void (QByteArray&)> unpack_func);
    void timerEvent(QTimerEvent *event);
    virtual void receiveDataHandler();


protected:
    QByteArray m_byteReceiveBuffer;
    std::function<void (QByteArray& buffer)> pUnpackFunc = nullptr;
};

class TransmitBase : public QObject
{
    Q_OBJECT
public:
    TransmitBase(uint16_t period_time = 10, QObject* parent = nullptr);
    void transmitDataBase(const QByteArray& data);
    void timerEvent(QTimerEvent *event);
    virtual void transmitDataHandler();

signals:
    void readyTransmit(QByteArray& buffer);

protected:
    QByteArray m_byteTransmitBuffer;
};

#endif // COMM_BASE_H
