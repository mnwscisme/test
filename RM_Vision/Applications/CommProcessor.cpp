#include "CommProcessor.h"
#include "CRC.h"

#include <QSettings>
#include <QtEndian>
#include "Common.h"

extern QString g_strRootDir;

ReceiveProcessor::ReceiveProcessor(uint8_t header_sof) :
    ReceiveBase(std::bind(&ReceiveProcessor::unpack, this, std::placeholders::_1), 5)
{
    nHeaderSof = header_sof;
}

void ReceiveProcessor::unpack(QByteArray& buffer)
{
    uint8_t byte = 0;
    while (!buffer.isEmpty())
    {
        byte = buffer.data()[0];
        buffer.remove(0, 1);
        switch (nUnpackStep)
        {
            case STEP_HEADER_SOF:
            {
                if (byte == nHeaderSof)
                {
                    nUnpackStep = STEP_LENGTH_LOW;
                    byteProtocolPacket.append(byte);
                }
                else
                {
                    byteProtocolPacket.clear();
                }
            }break;
            case STEP_LENGTH_LOW:
            {
                nDataLen = byte;
                nUnpackStep = STEP_LENGTH_HIGH;
                byteProtocolPacket.append(byte);
            }break;
            case STEP_LENGTH_HIGH:
            {
                nDataLen |= (byte << 8);
                byteProtocolPacket.append(byte);

                if (nDataLen < (PROTOCOL_FRAME_MAX_SIZE - HEADER_CRC_CMDID_LEN))
                {
                    nUnpackStep = STEP_FRAME_SEQ;
                }
                else
                {
                    nUnpackStep = STEP_HEADER_SOF;
                    byteProtocolPacket.clear();
                }
            }break;
            case STEP_FRAME_SEQ:
            {
                nUnpackStep = STEP_HEADER_CRC8;
                byteProtocolPacket.append(byte);
            }break;
            case STEP_HEADER_CRC8:
            {
                byteProtocolPacket.append(byte);

                if (byteProtocolPacket.size() == PROTOCOL_HEADER_SIZE)
                {
                    if (verify_crc8_check_sum((uint8_t*)byteProtocolPacket.data(), PROTOCOL_HEADER_SIZE))
                    {
                        nUnpackStep = STEP_DATA_CRC16;
                    }
                    else
                    {
                        nUnpackStep = STEP_HEADER_SOF;
                        byteProtocolPacket.clear();
                    }
                }
                else
                {
                    byteProtocolPacket.clear();
                }
            }break;
            case STEP_DATA_CRC16:
            {
                if (byteProtocolPacket.size() < (int)(HEADER_CRC_CMDID_LEN + nDataLen))
                {
                    byteProtocolPacket.append(byte);
                }
                if (byteProtocolPacket.size() >= (int)(HEADER_CRC_CMDID_LEN + nDataLen))
                {
                    nUnpackStep = STEP_HEADER_SOF;

                    if (verify_crc16_check_sum((uint8_t*)byteProtocolPacket.data(), (int)(HEADER_CRC_CMDID_LEN + nDataLen)))
                    {
                        FrameHeader_t *p_header = (FrameHeader_t*)byteProtocolPacket.data();
                        uint16_t data_length = p_header->data_length;
                        uint16_t cmd_id      = *(uint16_t *)(byteProtocolPacket.data() + PROTOCOL_HEADER_SIZE);
                        uint8_t* data_addr   = (uint8_t *)(byteProtocolPacket.data() + PROTOCOL_HEADER_SIZE + PROTOCOL_CMD_SIZE);

//                        if(pHookFunc != nullptr)
//                        {
//                            pHookFunc(cmd_id, data_addr, data_length);
//                        }
                        emit sigHookFunc(cmd_id, data_addr, data_length);
                    }
                    byteProtocolPacket.clear();
                }
            }break;
            default:
            {
                nUnpackStep = STEP_HEADER_SOF;
            }break;
        }
    }
}

TransmitProcessor::TransmitProcessor() :
    TransmitBase(5)
{
    nSeq = 0;
}

void TransmitProcessor::transmitData(uint8_t header_sof, uint16_t cmd_id, uint8_t* data, uint16_t len)
{
    uint8_t protocol_packet[PROTOCOL_FRAME_MAX_SIZE] = {0};
    uint16_t frame_length = PROTOCOL_HEADER_SIZE + PROTOCOL_CMD_SIZE + len + PROTOCOL_CRC16_SIZE;
    if (frame_length > PROTOCOL_FRAME_MAX_SIZE)
        return;
    FrameHeader_t *p_header = (FrameHeader_t*)(protocol_packet);

    p_header->sof          = header_sof;
    p_header->data_length  = len;
    p_header->seq          = 0;
    memcpy(protocol_packet + PROTOCOL_HEADER_SIZE, (uint8_t*)&cmd_id, PROTOCOL_CMD_SIZE);
    append_crc8_check_sum(protocol_packet, PROTOCOL_HEADER_SIZE);
    memcpy(protocol_packet + PROTOCOL_HEADER_SIZE + PROTOCOL_CMD_SIZE, data, len);
    append_crc16_check_sum(protocol_packet, frame_length);

    transmitDataBase(QByteArray((char*)protocol_packet, frame_length));
}

CommProcessor::~CommProcessor()
{
    RELEASE_ALLOC_MEM(m_Uart);
    RELEASE_ALLOC_MEM(m_UartRx);
    RELEASE_ALLOC_MEM(m_UartTx);
}

void CommProcessor::getVisionObj(VisionProcessor *vision)
{
    m_objVision = vision;
    connect(m_objVision, &VisionProcessor::sigSendVisionResult, this, &CommProcessor::sendVisionResult);
}

void CommProcessor::sendVisionResult(const VisionResult_t res)
{
    m_VisionResult = res;
    m_UartTx->transmitData(VISION_PROTOCOL_HEADER_SOF, VISION_DATA_CMD_ID, (uint8_t*)&m_VisionResult, sizeof(VisionResult_t));
}

void CommProcessor::ReceiveHook(uint16_t cmd_id, uint8_t* data, uint16_t len)
{
//    qDebug("color3:: %d",cmd_id);

    VisionControl_t ctr;
    switch (cmd_id)
    {
        case VISION_DATA_CMD_ID:
        {
        }break;
        case ROBOT_DATA_CMD_ID:
        {
            memcpy(&ctr, data, sizeof(VisionControl_t));
//            if (ctr.detect_target != m_VisionControl.detect_target ||
//                    ctr.enemy_color != m_VisionControl.enemy_color ||
//                    ctr.shoot_speed != m_VisionControl.shoot_speed)
            {
                m_VisionControl = ctr;
                if (m_objVision != nullptr)
                {
                    m_objVision->setDetectTarget(m_VisionControl.detect_target);
                    m_objVision->setEnemyColor(m_VisionControl.enemy_color);
                    m_objVision->setShootSpeed(m_VisionControl.shoot_speed);

                    qDebug("color2:: %d",m_VisionControl.enemy_color);
                }
            }
        }break;
        default:
            break;
    }


}

void CommProcessor::timerEvent(QTimerEvent* event)
{
    m_UartTx->transmitData(VISION_PROTOCOL_HEADER_SOF, VISION_DATA_CMD_ID, (uint8_t*)&m_VisionResult, sizeof(VisionResult_t));
}

CommProcessor::CommProcessor() :
    m_objVision(nullptr)
{
    readSettings();
    m_Uart = new UART_Handle(m_strUartName, m_u32BaudRate);
    m_UartRx = new ReceiveProcessor(VISION_PROTOCOL_HEADER_SOF);
    m_UartTx = new TransmitProcessor();
    m_Uart->bindReceive(m_UartRx);
    m_Uart->bindTransmit(m_UartTx);
    connect(m_UartRx, &ReceiveProcessor::sigHookFunc, this, &CommProcessor::ReceiveHook, Qt::DirectConnection);
//    startTimer(50);
}

void CommProcessor::readSettings()
{
    QSettings *set = new QSettings(g_strRootDir + "/Config.ini", QSettings::IniFormat);
    set->beginGroup("Comm");
#ifdef Q_OS_WIN32
    m_strUartName = set->value("UART", "COM1").toString();
#else
    m_strUartName = set->value("UART", "/dev/ttyTHS0").toString();
#endif
    m_u32BaudRate = set->value("BaudRate", 115200).toInt();
    set->endGroup();
}



