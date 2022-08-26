#ifndef COMM_PROC_H
#define COMM_PROC_H

#include "UartHandle.h"
#include "CommBase.h"
#include "Singleton.h"
#include "Protocol.h"
#include "VisionProcessor.h"

typedef enum
{
    STEP_HEADER_SOF  = 0,
    STEP_LENGTH_LOW  = 1,
    STEP_LENGTH_HIGH = 2,
    STEP_FRAME_SEQ   = 3,
    STEP_HEADER_CRC8 = 4,
    STEP_DATA_CRC16  = 5,
} UnpackStep_e;

#pragma pack(push,1)
typedef struct
{
    uint8_t  sof;
    uint16_t data_length;
    uint8_t  seq;
    uint8_t  crc8;
} FrameHeader_t;
#pragma pack(pop)

#define PROTOCOL_FRAME_MAX_SIZE         128
#define PROTOCOL_HEADER_SIZE            sizeof(FrameHeader_t)
#define PROTOCOL_CMD_SIZE               2
#define PROTOCOL_CRC16_SIZE             2
#define HEADER_CRC_LEN                  (PROTOCOL_HEADER_SIZE + PROTOCOL_CRC16_SIZE)
#define HEADER_CRC_CMDID_LEN            (PROTOCOL_HEADER_SIZE + PROTOCOL_CRC16_SIZE + sizeof(uint16_t))
#define HEADER_CMDID_LEN                (PROTOCOL_HEADER_SIZE + sizeof(uint16_t))

class ReceiveProcessor : public ReceiveBase
{
    Q_OBJECT
public:
    ReceiveProcessor(uint8_t header_sof);
    void unpack(QByteArray& buffer);
signals:
    void sigHookFunc(uint16_t, uint8_t*, uint16_t);

private:
    uint8_t nHeaderSof;
    UnpackStep_e nUnpackStep = STEP_HEADER_SOF;
    uint16_t nDataLen;
    QByteArray byteProtocolPacket;
};

class TransmitProcessor : public TransmitBase
{
public:
    TransmitProcessor();
    void transmitData(uint8_t header_sof, uint16_t cmd_id, uint8_t* data, uint16_t len);

private:
    uint8_t nSeq;
};

class CommProcessor : public QObject
{
    Q_OBJECT
    DECLARE_SINGLETON(CommProcessor);
public:
    ~CommProcessor();
    void getVisionObj(VisionProcessor* vision);
    void sendVisionResult(const VisionResult_t res);
signals:

private:
    void timerEvent(QTimerEvent *event);
    CommProcessor();
    void readSettings();
    void ReceiveHook(uint16_t cmd_id, uint8_t *data, uint16_t len);

    QString             m_strUartName;
    qint32              m_u32BaudRate;
    UART_Handle*        m_Uart;
    ReceiveProcessor*   m_UartRx;
    TransmitProcessor*  m_UartTx;
    VisionControl_t     m_VisionControl;
    VisionResult_t      m_VisionResult;
    VisionProcessor*    m_objVision;
};


#endif // COMM_PROC_H
