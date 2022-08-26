#ifndef ARMORDETECTOR_H
#define ARMORDETECTOR_H

#include "opencv.hpp"
#include "AngleSolver.hpp"
#include "Common.h"
#include "armorbox.h"

#define TRUNC_ABS(a) ((a) > 0 ? (a) : 0);
#define POINT_DIST(p1,p2) std::sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y))




enum EnemyColor { RED = 0, BLUE = 1};

// 很多成员都已经弃用了
struct ArmorParam {
    int min_light_height;			// 板灯最小高度值
    int light_slope_offset;		// 允许灯柱偏离垂直线的最大偏移量，单位度
    int max_light_delta_h;         // 左右灯柱在水平位置上的最大差值，像素单位
    int min_light_delta_h;		// 左右灯柱在水平位置上的最小差值，像素单位
    int max_light_delta_v;		// 左右灯柱在垂直位置上的最大差值，像素单位
    int max_light_delta_angle;	// 左右灯柱在斜率最大差值，单位度
    int near_face_v;            // 贴脸的距离
    float max_lr_rate;              // 左右灯柱的比例值
    float max_wh_ratio;         // 装甲板的最大长宽比
    float min_wh_ratio;         // 最小长宽比
    float small_armor_wh_threshold;// 大小装甲的界限阈值
    int bin_cls_thres;          // 二分类防止三根灯条的分割阈值
    float target_max_angle;

    // 默认的构造值
    ArmorParam(){
        min_light_height = 10;
        light_slope_offset = 30;
        max_light_delta_h = 720;
        min_light_delta_h = 20;
        max_light_delta_v = 100;
        max_light_delta_angle = 30;
        near_face_v = 600;
        max_lr_rate = 1.5;
        max_wh_ratio = 5.2;
        min_wh_ratio = 1.25;
        small_armor_wh_threshold = 3.6;
        bin_cls_thres = 166;
        target_max_angle = 20;
    }
};


typedef enum
{
    Gray = 0,
    Red = 1,
    Blue = 2,
    AllColor = Red|Blue,
} EnemyColor_e;


// 匹配灯条的结构体
struct matched_rect{
    cv::RotatedRect rect;
    float lr_rate;
    float angle_abs;
    cv::Point2f points[4];
};

//候选灯条的结构体
struct candidate_target{
    int armor_height;
    float armor_angle;
    int index;
    bool is_small_armor;
    float bar_lr_rate;
    float bar_angle_abs;
};


class ArmorDetector
{
public:
    enum DebugShow
    {
        NOT_SHOW = 0,
        SHOW_BINARY,
        FIRST_RESULT,
        SECOND_RESULT
    };



    ArmorDetector(const ArmorParam & para = ArmorParam());
    ~ArmorDetector();
    void setImage(const cv::Mat & src);
    const cv::Mat &getShowImage(uint8_t show_index);
    void findTargetInContours(std::vector<matched_rect> &match_rects);
    void chooseTarget(const std::vector<matched_rect> &match_rects, const cv::Mat &src, ArmorBox &target_armor);
    void getTargetAera(const cv::Mat &src, ArmorBox &target_armor);

    int enemy_color;
    uint8_t red_thres;
    uint8_t blue_thres;

    bool is_lost;
    int _lost_cnt;                  // 失去目标的次数，n帧没有识别到则全局搜索

private:
    bool PreProcess(const cv::Mat src, cv::Mat &binary, int binary_mode);
    bool makeRectSafe(cv::Rect &rect, cv::Size size);
    bool broadenRect(cv::Rect &rect, int width_added, int height_added, cv::Size size);
    cv::RotatedRect boundingRRect(const cv::RotatedRect &left, const cv::RotatedRect &right);
    void boundingRPoints(const cv::RotatedRect & left, const cv::RotatedRect & right, cv::Point2f points[]);

    ArmorParam _para;   // parameter of alg

    cv::Mat _src;      // source image
    cv::Size _size;    // 源图的尺寸大小
    cv::Mat _binary;   // binary image of sub between blue and red component
    cv::Mat _show;
    uint8_t show_d;

    cv::RotatedRect _res_last;      // last detect result
    cv::Rect _dect_rect;            // detect roi of original image

};

#endif // ARMORDETECTOR_H
