#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QtGlobal>

typedef enum
{
    RED_HERO        = 1,    //红方英雄
    RED_ENGINEER    = 2,    //红方工程
    RED_INFANTRY_1  = 3,    //红方步兵1
    RED_INFANTRY_2  = 4,    //红方步兵2
    RED_INFANTRY_3  = 5,    //红方步兵3
    RED_AERIAL      = 6,    //红方飞机
    RED_SENTRY      = 7,    //红方哨兵
    RED_DARTS       = 8,    //红方飞镖
    RED_RADAR       = 9,    //红方雷达
    BLUE_HERO       = 101,  //蓝方英雄
    BLUE_ENGINEER   = 102,  //蓝方工程
    BLUE_INFANTRY_1 = 103,  //蓝方步兵1
    BLUE_INFANTRY_2 = 104,  //蓝方步兵2
    BLUE_INFANTRY_3 = 105,  //蓝方步兵3
    BLUE_AERIAL     = 106,  //蓝方飞机
    BLUE_SENTRY     = 107,  //蓝方哨兵
    BLUE_DARTS      = 108,  //蓝方飞镖
    BLUE_RADAR      = 109,  //蓝方雷达
    WINDMILL        = 200,
} Robot_e;

#define VISION_PROTOCOL_HEADER_SOF 0x55

typedef enum
{
    VISION_DATA_CMD_ID   = 0x0001,
    ROBOT_DATA_CMD_ID    = 0x0002
} VISION_CMD_ID_e;

typedef enum
{
    VISION_TRACK_LOSS   = 0x0001,
    VISION_TRACK        = 0x0002
} VisionState_e;


typedef enum
{
    Hero        = 0x0001,
    Engineer    = 0x0002,
    Infantry    = 0x0004,
    Sentey      = 0x0008,
    AllRobot = Hero|Engineer|Infantry|Sentey,
    Windmill    = 0x1000
} DetectTarget_e;

#pragma pack(push,1)
typedef struct
{
    uint16_t detect_target;
    uint16_t enemy_color;
    uint8_t shoot_speed;

} VisionControl_t;

typedef struct
{
    float pitch;
    float yaw;
    float distance;
    uint8_t state;  // VisionState_e VISION_TRACK
    uint8_t robot_num;
    uint8_t fire;
    uint8_t update;
} VisionResult_t;
#pragma pack(pop)


#endif // PROTOCOL_H
