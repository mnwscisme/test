#ifndef UART_HANDLE_H
#define UART_HANDLE_H

#include <iostream>
#include <functional>
#include <QSerialPort>
#include <QSerialPortInfo>
#include "CommBase.h"

class UART_Handle : public QObject
{
    Q_OBJECT
public:
    UART_Handle(QObject* parent = nullptr);
    UART_Handle(QString name, qint32 baud_rate, QObject* parent = nullptr);

    bool open(QString name, qint32 baud_rate);
    bool isOpen();
    void close();
    qint64 write(const QByteArray &byte_array);
    qint64 write(char *data, qint64 len);
    void bindReceive(ReceiveBase* receive);
    void bindTransmit(TransmitBase* transmit);

signals:
    void readyRead();
    void errorOccurred(QSerialPort::SerialPortError error);

protected:
    void receiveHandler();
    void transmitHandler(QByteArray &byte_array);

private:
    QSerialPort* m_pSerial = nullptr;
    ReceiveBase* m_pReceive = nullptr;
    TransmitBase* m_pTransmit = nullptr;
};

#endif // UART_HANDLE_H
