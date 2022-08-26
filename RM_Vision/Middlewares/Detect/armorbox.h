#ifndef ARMORBOX_H
#define ARMORBOX_H

#include "opencv.hpp"

#define BIG_ARMOR_HEIGHT    127.0 //mm
#define BIG_ARMOR_WIDTH     230.0 //mm
#define SMALL_ARMOR_HEIGHT  125.0 //mm
#define SMALL_ARMOR_WIDTH   135.0 //mm
#define WINDMILL_ARMOR_HEIGHT 120.0 //mm
#define WINDMILL_ARMOR_WIDTH 120.0 //mm

#define MAX_DISTANCE 99999  //
#define DISTANCE_LIMIT 1000 //mm

typedef enum
{
    SmallArmor = 0,
    BigArmor = 1,
    WindmillArmor = 2,
} ArmorType_e;

//typedef struct
//{
//    ArmorType_e     type;
//    int             num;        //装甲板上的数字
//    EnemyColor_e    color;
//    Rect            rect;       //装甲板的矩形获取roi用
//    Point2f         center;     //装甲板中心
//    float           angle;      //装甲板角度
//    Point2f         l_light_p[4], r_light_p[4];  //装甲板的左右灯条
//    Point2f         points[4];

//    float pitch;
//    float yaw;
//    float distance;
//}ArmorBox;

class ArmorBox
{
public:
    ArmorBox()
    {
        type = SmallArmor;
//        color = AllColor;
        distance = MAX_DISTANCE;
        rect = cv::RotatedRect();
        pitch = 0.0;
        yaw = 0.0;
    }

    ArmorType_e     type;
    cv::RotatedRect rect;       //装甲板的矩形获取roi用
    cv::Point2f     points[4];

    float pitch;
    float yaw;
    float distance;
};


#endif // ARMORBOX_H
