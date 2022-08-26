#include "UartHandle.h"
#include <QDebug>

UART_Handle::UART_Handle(QObject* parent) : QObject(parent)
{

}

UART_Handle::UART_Handle(QString name, qint32 baud_rate, QObject* parent) : QObject(parent)
{
    open(name, baud_rate);
}

bool UART_Handle::open(QString name, qint32 baud_rate)
{
    close();

    m_pSerial = new QSerialPort(this);
    m_pSerial->setPortName(name);
    m_pSerial->setBaudRate(baud_rate);
    m_pSerial->setDataBits(QSerialPort::Data8);
    m_pSerial->setParity(QSerialPort::NoParity);
    m_pSerial->setStopBits(QSerialPort::OneStop);
    m_pSerial->setFlowControl(QSerialPort::NoFlowControl);

    if (m_pSerial->open(QIODevice::ReadWrite))
    {
        connect(m_pSerial, &QSerialPort::readyRead, this, &UART_Handle::readyRead);
        connect(m_pSerial, &QSerialPort::errorOccurred, this, &UART_Handle::errorOccurred);
        qDebug("Serial port %s opened successfully, baud rate: %d.", qPrintable(name), baud_rate);
        return true;
    }
    else
    {
        qDebug("Serial port %s opened failed.", qPrintable(name));
        return false;
    }
}


bool UART_Handle::isOpen()
{
    bool rec = (m_pSerial != nullptr && m_pSerial->isOpen());
    if (!rec)
    {
        qDebug("m_pSerial == nullptr or not open");
    }
    return rec;
}

void UART_Handle::close()
{
    if (m_pSerial != nullptr)
    {
        m_pSerial->close();
        delete m_pSerial;
        m_pSerial = nullptr;
    }
}

void UART_Handle::receiveHandler()
{
    if (isOpen())
    {
        if (m_pReceive != nullptr)
        {
            m_pReceive->receiveData(m_pSerial->readAll());
        }
        else
        {
            qDebug("Receive not bind");
        }
    }
}

void UART_Handle::transmitHandler(QByteArray& byte_array)
{
    if (isOpen())
    {
        if (m_pTransmit != nullptr)
        {
            uint16_t size = write(byte_array);
        }
        else
        {
            qDebug("Transmit not bind");
        }
    }
}

qint64 UART_Handle::write(const QByteArray &byte_array)
{
    if (isOpen())
    {
        return m_pSerial->write(byte_array);
    }
    return 0;
}

qint64 UART_Handle::write(char *data, qint64 len)
{
    if (isOpen())
    {
        return m_pSerial->write(data, len);
    }
    return 0;
}

void UART_Handle::bindReceive(ReceiveBase* receive)
{
    Q_ASSERT(receive);
    m_pReceive = receive;
    connect(m_pSerial, &QSerialPort::readyRead, this, &UART_Handle::receiveHandler);
}

void UART_Handle::bindTransmit(TransmitBase* transmit)
{
    Q_ASSERT(transmit);
    m_pTransmit = transmit;
    connect(m_pTransmit, &TransmitBase::readyTransmit, this, &UART_Handle::transmitHandler);
}

