#include "ArmorDetector.h"
#include "ArmorDetector.h"

//#define SHOW_DEBUG_IMG
//#define COUT_LOG
//#define CLASSIFICATION

using namespace cv;
using namespace std;

enum BinaryMode
{
    RGB = 1,
    HSV = 2,
    RGB2G = 3,
    OTSU = 4,
    GRAY = 5,
    YCrCb = 6,
    LUV = 7
};

/**
 *@brief: calculate the cross point of four points in order bl(below left),tl(top left),tr(top right),br(below right)
 */
const Point2f crossPointof(const Point2f& bl,const Point2f& tl,const Point2f& tr, const Point2f& br) {
    float a1 = tr.y - bl.y;
    float b1 = tr.x - bl.x;
    float c1 = bl.x*tr.y - tr.x*bl.y;

    float a2 = br.y - tl.y;
    float b2 = br.x - tl.x;
    float c2 = tl.x*br.y - br.x*tl.y;

    float d = a1 * b2 - a2 * b1;

    if (d == 0.0){
        return Point2f(FLT_MAX, FLT_MAX);
    }
    else{
        return cv::Point2f((b2*c1 - b1 * c2) / d, (c1*a2 - c2 * a1) / d);
    }
}


bool ArmorDetector::PreProcess(const Mat src, Mat& binary, int binary_mode)
{
    if (src.empty() || src.channels() != 3) return false;

    Mat gray,gray_binary,tempBinary;

    if (binary_mode == RGB)
    {
        cvtColor(src, gray, COLOR_RGB2GRAY);
        threshold(gray,gray_binary,50,255,THRESH_BINARY);

        // 红蓝通道相减
        std::vector<Mat> splited;
        split(src,splited);
        if(enemy_color == Red){
           subtract(splited[0],splited[2],tempBinary);
           threshold(tempBinary,tempBinary, red_thres,255,THRESH_BINARY); //60
        }else if(enemy_color == Blue ){
           subtract(splited[2],splited[0],tempBinary);
           threshold(tempBinary,tempBinary, blue_thres,255,THRESH_BINARY); //135
        }else{
            return false;
        }
        dilate(tempBinary,tempBinary,getStructuringElement(MORPH_RECT,Size(5,5)));
        // mask 操作
        binary = tempBinary & gray_binary;
    }
    else if (binary_mode == HSV)
    {
        // 亮度图
        cvtColor(src,gray,COLOR_RGB2GRAY);
        threshold(gray,gray_binary, 80, 255,THRESH_BINARY); //80

        // 颜色阈值分割
        Mat imgHSV;
        cvtColor(src,imgHSV,COLOR_RGB2HSV);
        if(enemy_color == Red){
            Mat temp;
            inRange(imgHSV,Scalar(0,60,80),Scalar(25,255,255),temp);
            inRange(imgHSV,Scalar(156,60,80),Scalar(181,255,255),tempBinary);
            tempBinary = temp | tempBinary;
        }else if(enemy_color == Blue){
            inRange(imgHSV,Scalar(35,46,80),Scalar(99,255,255),tempBinary);
        }else{
            return false;
        }
        dilate(tempBinary,tempBinary,getStructuringElement(MORPH_RECT,Size(3,3)));
        // mask 操作
        binary = tempBinary & gray_binary;
    }
    else if(binary_mode == RGB2G)
    {
        cvtColor(src,gray,COLOR_RGB2HSV);
        threshold(gray,gray_binary,80,255,THRESH_BINARY);

        // 与绿通道相减
        std::vector<Mat> splited;
        split(src,splited);
        if(enemy_color == Red){
           subtract(splited[2],splited[1],tempBinary);
           threshold(tempBinary,tempBinary,red_thres,255,THRESH_BINARY);
        }else if(enemy_color == Blue){
           subtract(splited[0],splited[1],tempBinary);
           threshold(tempBinary,tempBinary,blue_thres,255,THRESH_BINARY);
        }else{
            return false;
        }

        dilate(tempBinary,tempBinary,getStructuringElement(MORPH_RECT,Size(3,3)));
        // mask 操作
        binary = tempBinary & gray_binary;
    }
    else if(binary_mode == OTSU)
    {
        // 大津算法
        cvtColor(src,gray,COLOR_BGR2GRAY);
        double test = threshold(gray,tempBinary,0,255,THRESH_OTSU);// 可以得出一个阈值
        binary = tempBinary;
    }
    else if(binary_mode == GRAY)
    {
        // 灰度阈值
        cvtColor(src,gray,COLOR_BGR2GRAY);
        threshold(gray,gray_binary,40,255,THRESH_BINARY);
        binary = gray_binary;
    }
//    else if(binary_mode == YCrCb)
//    {

//        Mat Ycrcb;
//        cvtColor(src,Ycrcb,COLOR_BGR2YCrCb);
//        std::vector<Mat> splited;
//        split(Ycrcb,splited);

//        // 亮度图
//        threshold(splited[0],gray_binary,60,255,THRESH_BINARY);

//        // cr和cb通道
//        if(enemy_color == Red){
//            subt_max_colorract(splited[1],splited[2],tempBinary);
//            threshold(tempBinary,tempBinary,20,255,THRESH_BINARY);
//        }else if(enemy_color == Blue){
//            subtract(splited[2],splited[1],tempBinary);
//            threshold(tempBinary,tempBinary,40,255,THRESH_BINARY);
//        }else{
//            return false;
//        }

//        dilate(tempBinary,tempBinary,getStructuringElement(MORPH_RECT,Size(3,3)));
//        // mask 操作
//        binary = tempBinary & gray_binary;
//    }
    else if(binary_mode == LUV)
    {
        Mat luv;
        cvtColor(src,luv,COLOR_BGR2Luv);
        std::vector<Mat> splited;
        split(luv,splited);

        // 亮度图
        threshold(splited[0],gray_binary,60,255,THRESH_BINARY);

        // 颜色阈值
        if(enemy_color == Red){
            threshold(splited[2],tempBinary,160,255,THRESH_BINARY);
        }else if(enemy_color == Blue){
            threshold(splited[1],tempBinary,70,255,THRESH_BINARY_INV);

        }else{
            return false;
        }
        dilate(tempBinary,tempBinary,getStructuringElement(MORPH_RECT,Size(3,3)));

        // mask操作
        binary = gray_binary & tempBinary;
    }else{
        return false;
    }

    return true;
}

/**
 * @brief makeRectSafe 使矩形不发生越界
 * @param rect 输入的矩形， x， y为左上角的坐标
 * @param size 限制的大小，防止越界
 * @return
 */
bool ArmorDetector::makeRectSafe(cv::Rect & rect, cv::Size size){
    if (rect.x < 0)
        rect.x = 0;
    if (rect.x + rect.width > size.width)
        rect.width = size.width - rect.x;
    if (rect.y < 0)
        rect.y = 0;
    if (rect.y + rect.height > size.height)
        rect.height = size.height - rect.y;
    if (rect.width <= 0 || rect.height <= 0)
        // 如果发现矩形是空的，则返回false
        return false;
    return true;
}

/**
 * @brief broadenRect 给矩形扩张，然后判断是否越界
 * @param rect        思想是左上角移动，也就是围绕中心扩张
 * @param width_added 扩张的宽 / 2
 * @param height_added 扩张的高/ 2
 * @param size
 * @return
 */
bool ArmorDetector::broadenRect(cv::Rect & rect, int width_added, int height_added, cv::Size size){
    rect.x -= width_added;
    rect.width += width_added * 2;
    rect.y -= height_added;
    rect.height += height_added * 2;
    return makeRectSafe(rect, size);
}

cv::RotatedRect ArmorDetector::boundingRRect(const cv::RotatedRect & left, const cv::RotatedRect & right){
    // 这个函数是用来将左右边的灯条拟合成一个目标旋转矩形，没有考虑角度
    const Point & pl = left.center, & pr = right.center;
    Point2f center = (pl + pr) / 2.0;
//    cv::Size2f wh_l = left.size;
//    cv::Size2f wh_r = right.size;
    // 这里的目标矩形的height是之前灯柱的width
    double width_l = MIN(left.size.width, left.size.height);
    double width_r = MIN(right.size.width, right.size.height);
    double height_l = MAX(left.size.width, left.size.height);
    double height_r = MAX(right.size.width, right.size.height);
    float width = POINT_DIST(pl, pr) - (width_l + width_r) / 2.0;
    float height = std::max(height_l, height_r);
    //float height = (wh_l.height + wh_r.height) / 2.0;
    float angle = std::atan2(right.center.y - left.center.y, right.center.x - left.center.x);
    return RotatedRect(center, Size2f(width, height), angle * 180 / CV_PI);
}

void ArmorDetector::boundingRPoints(const cv::RotatedRect & left, const cv::RotatedRect & right, Point2f points[4]){
    double width_l = MIN(left.size.width, left.size.height);
    double width_r = MIN(right.size.width, right.size.height);
    double height_l = MAX(left.size.width, left.size.height);
    double height_r = MAX(right.size.width, right.size.height);
    float l_angle = fabs(left.angle);
    float r_angle = fabs(right.angle);
    if (l_angle > 45)
        l_angle -= 90;
    if (r_angle > 45)
        r_angle -= 90;
//    float angle = (l_angle +  r_angle) / 2;
    cv::Size exLSize(int(width_l), int(height_l*2));
    cv::Size exRSize(int(width_r), int(height_r*2));
    cv::RotatedRect exLLight(left.center, exLSize, l_angle);
    cv::RotatedRect exRLight(right.center, exRSize, r_angle);


    cv::Point2f pts_l[4];
    exLLight.points(pts_l);
    cv::Point2f pts_r[4];
    exRLight.points(pts_r);

//    if (show_d == FIRST_RESULT)
//    {
//        for (int i = 0; i < 4; i++)
//        {
//            cv::line(_show, pts_l[i], pts_l[(i + 1) % 4], Scalar(255, 0, 255), 1);
//            cv::line(_show, pts_r[i], pts_r[(i + 1) % 4], Scalar(255, 0, 255), 1);
//            cv::putText(_show, QString::number( i).toStdString(),
//                        pts_l[i], cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255, 0, 255), 2);
//            cv::putText(_show, QString::number( i).toStdString(),
//                        pts_r[i], cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(255, 0, 255), 2);
//        }
//    }

    points[0] = pts_r[2];
    points[1] = pts_r[3];
    points[2] = pts_l[0];
    points[3] = pts_l[1];
}

ArmorDetector::ArmorDetector(const ArmorParam & para)
{
    _para = para;
    _res_last = cv::RotatedRect();
    _dect_rect = cv::Rect();
    show_d = NOT_SHOW;
}


ArmorDetector::~ArmorDetector()
{

}

void ArmorDetector::setImage(const Mat &src)
{
    _size = src.size();

    // 注意这个_res_last是一个旋转矩形
    const cv::Point & last_result = _res_last.center;

    // 如果上一次的目标没了，源图就是输入的图
    if(last_result.x == 0 || last_result.y == 0)
    {
        _src = src;
        _dect_rect = Rect(0, 0, src.cols, src.rows);
    }
    else
    {
        // 如果上一次的目标没有丢失的话，用直立矩形包围上一次的旋转矩形
        Rect rect = _res_last.boundingRect();
        // _para.max_light_delta_h 是左右灯柱在水平位置上的最大差值，像素单位
        int max_half_w = _para.max_light_delta_h * 1.3;
        int max_half_h = 300;


        // 截图的区域大小。太大的话会把45度识别进去
        double scale_w = 1.3 + 0.7;
        double scale_h = 2;

        int w = int(rect.width * scale_w);
        int h = int(rect.height * scale_h);
        Point center = last_result;
        int x = std::max(center.x - w, 0);
        int y = std::max(center.y - h, 0);
        Point lu = Point(x, y);  /* point left up */
        x = std::min(center.x + w, src.cols);
        y = std::min(center.y + h, src.rows);
        Point rd = Point(x, y);  /* point right down */

        // 构造出矩形找到了搜索的ROI区域
        _dect_rect = Rect(lu, rd);
        // 为false说明矩形是空的，所以继续搜索全局像素
        // 感觉这里会有点bug
        // 矩形是空的则返回false
        if (makeRectSafe(_dect_rect, src.size()) == false){
            _res_last = cv::RotatedRect();
            _dect_rect = Rect(0, 0, src.cols, src.rows);
            _src = src;
        }
        else
            // 如果ROI矩形合法的话就用此ROI
            src(_dect_rect).copyTo(_src);
    }

    //==========================上面已经设置好了真正处理的原图============================

    // 下面是在敌方通道上二值化进行阈值分割
    // _max_color是红蓝通道相减之后的二值图像
    ///////////////////////////// begin /////////////////////////////////////////////
    /**
     * 预处理其实是最重要的一步，这里有HSV和RGB两种预处理的思路，其实大致都差不多
     * 只不过在特定场合可以选择特定的预处理方式
     * 例如HSV的话可以完全过滤掉日光灯的干扰，但是耗时较大
     */
    _binary = cv::Mat(_src.size(), CV_8UC1, cv::Scalar(0));

    PreProcess(_src, _binary, RGB);
    if (show_d == SHOW_BINARY)
    {
        _binary.copyTo(_show);
    }

}


const Mat &ArmorDetector::getShowImage(uint8_t show_index)
{
    if (show_index != show_d)
    {
        show_d = show_index;
        return _src;
    }
    return _show;
}


void ArmorDetector::findTargetInContours(vector<matched_rect> & match_rects)
{
    // find contour in sub image of blue and red
    vector<vector<Point2i> > contours_max;
    vector<Vec4i> hierarchy;
    // for debug use
    if (show_d == FIRST_RESULT)
    {
        _src.copyTo(_show);
    }

    // br是直接在_max_color上寻找的轮廓
    findContours(_binary, contours_max, hierarchy, RETR_EXTERNAL , CHAIN_APPROX_SIMPLE);

    // 用直线拟合轮廓，找出符合斜率范围的轮廓
    vector<RotatedRect> RectFirstResult;
    for (size_t i = 0; i < contours_max.size(); ++i){
        // fit the lamp contour as a eclipse
        RotatedRect rrect = minAreaRect(contours_max[i]);
        double max_rrect_len = MAX(rrect.size.width, rrect.size.height);
        double min_rrect_len = MIN(rrect.size.width, rrect.size.height);

        /////////////////////////////// 单根灯条的条件 //////////////////////////////////////
        // 角度要根据实际进行略微改动
        bool if1 = (fabs(rrect.angle) < 45.0 && rrect.size.height > rrect.size.width); // 往左
        bool if2 = (fabs(rrect.angle) > 60.0 && rrect.size.width > rrect.size.height); // 往右
        bool if3 = max_rrect_len > _para.min_light_height; // 灯条的最小长度
        bool if4 = (max_rrect_len / min_rrect_len >= 1.1) && (max_rrect_len / min_rrect_len < 30); // 灯条的长宽比
        // 筛除横着的以及太小的旋转矩形 (本来是45的，后来加成60)
//        qDebug("%d %d %d %d", if1,if2,if3,if4);
        if ((if1 || if2) && if3 && if4)
        {
            RectFirstResult.push_back(rrect);
            if (show_d == 1)
            {
                Point2f vertice[4];
                rrect.points(vertice);
                for (int i = 0; i < 4; i++)  // 黄色
                    line(_show, vertice[i], vertice[(i + 1) % 4], Scalar(0, 255, 255), 2);
            }
        }
    }

    // 少于两根灯条就认为没有匹配到
    if (RectFirstResult.size() < 2){
        match_rects.size() == 0;
        return;
    }

    // 将旋转矩形从左到右排序
    sort(RectFirstResult.begin(), RectFirstResult.end(),
         [](RotatedRect & a1,RotatedRect & a2){
        return a1.center.x < a2.center.x;});

    Point2f _pt[4],pt[4];
    auto ptangle = [](const Point2f &p1,const Point2f &p2){
        return fabs(atan2(p2.y-p1.y,p2.x-p1.x)*180.0/CV_PI);
    };

    ///////////////////////////////////// 匹配灯条的条件 //////////////////////////////////////////////////////
    // 两两比较，有符合条件的就组成一个目标旋转矩形
    for (size_t i = 0; i < RectFirstResult.size() - 1; ++i) {
        const RotatedRect & rect_i = RectFirstResult[i];
        const Point & center_i = rect_i.center;
        float xi = center_i.x;
        float yi = center_i.y;
        float leni = MAX(rect_i.size.width, rect_i.size.height);
        float anglei = fabs(rect_i.angle);
        rect_i.points(_pt);
         /* ptabs(
          * 0 2
          * 1 3
          * */
        // 往右斜的长灯条
        // rRect.points有顺序的，y最小的点是0,顺时针1 2 3
        if(anglei > 45.0){
            pt[0] = _pt[3];
            pt[1] = _pt[0];
        }
        // 往左斜的
        else{
            pt[0] = _pt[2];
            pt[1] = _pt[3];
        }

        for (size_t j = i + 1; j < RectFirstResult.size(); j++) {
            const RotatedRect & rect_j = RectFirstResult[j];
            const Point & center_j = rect_j.center;
            float xj = center_j.x;
            float yj = center_j.y;
            float lenj = MAX(rect_j.size.width, rect_j.size.height);
            float anglej=  fabs(rect_j.angle);
            float delta_h = xj - xi;
            float lr_rate = leni > lenj ? leni / lenj : lenj / leni;
            float angleabs;


            rect_j.points(_pt);
            if(anglej > 45.0){
                pt[2] = _pt[2];
                pt[3] = _pt[1];
            }else{
                pt[2] = _pt[1];
                pt[3] = _pt[0];
            }
            double maxangle = MAX(ptangle(pt[0],pt[2]),ptangle(pt[1],pt[3]));
            //std::cout<<"angle:"<<maxangle<<std::endl;
            // maxangle = 0;
            if(anglei > 45.0 && anglej < 45.0){ // 八字 / \   //
                angleabs = 90.0 - anglei + anglej;
            }else if(anglei <= 45.0 && anglej >= 45.0){ // 倒八字 \ /
                angleabs = 90.0 - anglej + anglei;
            }else{
                if(anglei > anglej) angleabs = anglei - anglej; // 在同一边
                else angleabs = anglej - anglei;
            }


            // if rectangle is m atch condition, put it in candidate vector
            // lr_rate1.3有点小了，调大点可以对付破掉的3号车
            bool condition1 = delta_h > _para.min_light_delta_h && delta_h < _para.max_light_delta_h;
            bool condition2 = MAX(leni, lenj) >= 113 ? abs(yi - yj) < 166\
                                                       && abs(yi - yj) < 1.66 * MAX(leni, lenj) :
                                                       abs(yi - yj) < _para.max_light_delta_v\
                                                       && abs(yi - yj) < 1.2 * MAX(leni, lenj); // && abs(yi - yj) < MIN(leni, lenj)
            bool condition3 = lr_rate < _para.max_lr_rate;
            bool condition4 = angleabs < 15 ; // 给大点防止运动时掉帧
//            bool condition4 = angleabs > 25 && angleabs < 55;
            //                bool condition5 = sentry_mode ? true : /*maxangle < 20*/true;

            //            bool condition4 = delta_angle < _para.max_light_delta_angle;

            Point text_center = Point((xi + xj) / 2, (yi + yj) / 2);
#ifdef COUT_LOG
            cout << "delta_h:  " << abs(yi - yj) << endl;
            cout << "lr rate:  " << lr_rate << endl;
            cout << "length:   " << MAX(leni, lenj) << endl;
            cout << condition1 << " " << condition2 << " " << condition3 << " " << condition4 << endl;
#endif
            if (show_d == FIRST_RESULT)
            {
                putText(_show, to_string(int(angleabs)), text_center, 3, 1, Scalar(0, 255, 0), 2);
                putText(_show, to_string(int(maxangle)), Point(text_center.x + 20, text_center.y + 20), 3, 1, Scalar(255, 0, 0), 2);

            }
//            qDebug("%d %d %d %d", condition1,condition2, condition3, condition4);
            if (condition1 && condition2 && condition3 && condition4){
                matched_rect mat_rect;
                mat_rect.rect = boundingRRect(rect_i, rect_j);

                double w = mat_rect.rect.size.width;
                double h = mat_rect.rect.size.height;
                double wh_ratio = w / h;
                // 长宽比不符
#ifdef COUT_LOG
                cout << "wh_ratio:  " << wh_ratio << endl;
#endif
                // 基地模式不受长宽比的限制
//                if (!base_mode){
                    if (wh_ratio > _para.max_wh_ratio || wh_ratio < _para.min_wh_ratio)
                        continue;
//                }

                mat_rect.lr_rate = lr_rate;
                mat_rect.angle_abs = angleabs;
                boundingRPoints(rect_i, rect_j, mat_rect.points);

                // 将初步匹配到的结构体信息push进入vector向量
                match_rects.push_back(mat_rect);
                // for debug use
                if (show_d == FIRST_RESULT)
                {
                    Point2f l_light_p[4], r_light_p[4];
                    rect_i.points(l_light_p);
                    rect_j.points(r_light_p);
                    for (int i = 0; i < 4; i++)
                    {
                        cv::line(_show, l_light_p[i], l_light_p[(i + 1) % 4], Scalar(0, 255, 0), 1);
                        cv::line(_show, r_light_p[i], r_light_p[(i + 1) % 4], Scalar(0, 255, 0), 1);
                        cv::putText(_show, QString::number( i).toStdString(),
                                    l_light_p[i], cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 255, 0), 2);
                        cv::putText(_show, QString::number( i).toStdString(),
                                    r_light_p[i], cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 255, 0), 2);

                        line(_show, mat_rect.points[i], mat_rect.points[(i + 1) % 4], Scalar(255, 255, 255), 2);
                        cv::putText(_show, QString::number( i).toStdString(),
                                   mat_rect.points[i], cv::FONT_HERSHEY_PLAIN, 1.2, cv::Scalar(0, 255, 0), 2);
                    }
                }
            }
        }
    }
}

void ArmorDetector::chooseTarget(const std::vector<matched_rect> & match_rects, const cv::Mat & src, ArmorBox& target_armor) {
    // 如果没有两条矩形围成一个目标装甲板就返回一个空的旋转矩形
    if (match_rects.size() < 1){
        is_lost = true;
        target_armor = ArmorBox();
        return;
    }


    // 初始化参数
    int ret_idx = -1;
    bool is_small = false;
    double weight = 0;
    vector<candidate_target> candidate;
    // 二分类判断真假装甲板初始化
    Mat input_sample;
    vector<Mat> channels;

    #define SafeRect(rect, max_size) {if (makeRectSafe(rect, max_size) == false) continue;}

    ///////////////////////// 匹配灯条 ////////////////////////////////////////////////
    //======================= 开始循环 ====================================

    for (size_t i = 0; i < match_rects.size(); ++i){
        const RotatedRect & rect = match_rects[i].rect;

        // 长宽比不符,担心角度出问题，可以侧着车身验证一下（上面一个函数好像写过这个条件了）
        double w = rect.size.width;
        double h = rect.size.height;
        double wh_ratio = w / h;


        // 如果矩形的w和h之比小于阈值的话就是小装甲，否则是大装甲(初步判断)
        if (wh_ratio < _para.small_armor_wh_threshold)
        {
            is_small = true;
        }
        else
        {
            is_small = false;
        }

//        cout << "wh_ratio: " << wh_ratio << endl;

        // 用均值和方差去除中间太亮的图片（例如窗外的灯光等）
        RotatedRect screen_rect = RotatedRect(rect.center, Size2f(rect.size.width * 0.88, rect.size.height), rect.angle);
        Size size = Point(src.cols, src.rows);
        Point p1, p2;
        int x = screen_rect.center.x - screen_rect.size.width / 2 + _dect_rect.x;
        int y = screen_rect.center.y - screen_rect.size.height / 2 + _dect_rect.y;
        p1 = Point(x, y);
        x = screen_rect.center.x + screen_rect.size.width / 2 + _dect_rect.x;
        y = screen_rect.center.y + screen_rect.size.height / 2 + _dect_rect.y;
        p2 = Point(x, y);
        Rect roi_rect = Rect(p1, p2);
        Mat roi;
        if(makeRectSafe(roi_rect, size)){
            roi = src(roi_rect).clone();
            Mat mean, stdDev;
            double avg, stddev;
            meanStdDev(roi, mean, stdDev);
            avg = mean.ptr<double>(0)[0];
            stddev = stdDev.ptr<double>(0)[0];


            // 阈值可通过实际测量修改
            if (avg > 66.66)
                continue;
        }

        // 现在这个旋转矩形的角度
        float cur_angle = match_rects[i].rect.size.width > match_rects[i].rect.size.height ? \
                    abs(match_rects[i].rect.angle) : 90 - abs(match_rects[i].rect.angle);
        // 现在这个旋转矩形的高度（用来判断远近）
        int cur_height = MIN(w, h);
        // 最终目标如果太倾斜的话就筛除
        if (cur_angle > 33 - 5) continue; // (其实可以降到26)
        // 把矩形的特征信息push到一个候选vector中
        candidate.push_back(candidate_target{cur_height, cur_angle, i, is_small, match_rects[i].lr_rate, match_rects[i].angle_abs});
        ret_idx = 1;
    }
    //================================ 到这里才结束循环 =======================================
    int final_index = 0;
        if (candidate.size() > 1){
        // 将候选矩形按照高度大小排序，选出最大的（距离最近）
        sort(candidate.begin(), candidate.end(),
             [](candidate_target & target1, candidate_target & target2){
            return target1.armor_height > target2.armor_height;
        });
        // 做比较，在最近的几个矩形中（包括45度）选出斜率最小的目标
        /**
         * 下面的几个temp值可以筛选出最终要击打的装甲板，我只用了一两个效果已经挺好的了
         * 只是偶尔还会有误识别的情况，可以将这几个temp值都组合起来进行最终的判断
         * */

        float temp_angle = candidate[0].armor_angle;
        float temp_lr_rate = candidate[0].bar_lr_rate;
        float temp_angle_abs = candidate[0].bar_angle_abs;
        float temp_weight = temp_angle + temp_lr_rate;

        for (int i = 1; i < candidate.size(); i++){
            double angle = match_rects[candidate[i].index].rect.angle;
            if ( candidate[0].armor_height / candidate[i].armor_height < 1.1){
                if (candidate[i].armor_angle < temp_angle
                        /*&& (candidate[i].bar_lr_rate */){
                    temp_angle = candidate[i].armor_angle;
                    if (candidate[i].bar_lr_rate < 1.66) final_index = i;
                }
            }
        }
    }

    if (show_d == SECOND_RESULT)
    {
        _src.copyTo(_show);
        //     候选区域
        Point2f vertices[4];
        match_rects[final_index].rect.points(vertices);
        putText(_show, to_string(int(match_rects[final_index].rect.angle)), match_rects[final_index].rect.center, 3, 1, Scalar(0, 255, 0), 2);
        for (int i = 0; i < 4; i++)
            line(_show, vertices[i], vertices[(i + 1) % 4], CV_RGB(255, 0, 0));
    }

    // ret_idx为 -1 就说明没有目标
    if (ret_idx == -1){
        is_lost = true;
        target_armor = ArmorBox();
        return ;
    }
    // 否则就证明找到了目标
    is_lost = false;
    if (candidate[final_index].is_small_armor)
        target_armor.type = SmallArmor;
    else
        target_armor.type = BigArmor;
    target_armor.rect = match_rects[candidate[final_index].index].rect;
    for (uint8_t i=0; i<4; i++)
        target_armor.points[i] = match_rects[candidate[final_index].index].points[i];
    return;
}


// 将之前的各个函数都包含在一个函数中，直接用这一个
void ArmorDetector::getTargetAera(const cv::Mat & src, ArmorBox& target_armor){
    setImage(src);  // function called
    vector<matched_rect> match_rects;
    findTargetInContours(match_rects);  // function called
    chooseTarget(match_rects, src, target_armor);  // function called
    RotatedRect& final_rect = target_armor.rect;

    if(final_rect.size.width != 0){
        // 最后才加上了偏移量，前面那些的坐标都是不对的，所以不能用前面那些函数解角度
        final_rect.center.x += _dect_rect.x;
        final_rect.center.y += _dect_rect.y;
        for (int pnp = 0; pnp<4; pnp++)
        {
            target_armor.points[pnp].x += _dect_rect.x;
            target_armor.points[pnp].y += _dect_rect.y;
        }
        _res_last = final_rect;
        _lost_cnt = 0;
    }
    else{
//        _find_cnt = 0;
        ++_lost_cnt;

        // 逐次加大搜索范围（根据相机帧率调整参数）
        if (_lost_cnt < 19)        //19
            _res_last.size = Size2f(_res_last.size.width, _res_last.size.height);
        else if(_lost_cnt == 20)   //20
            _res_last.size =Size2f(_res_last.size.width * 1.5, _res_last.size.height * 1.5);
        else if(_lost_cnt == 30)  //30
            _res_last.size = Size2f(_res_last.size.width * 2, _res_last.size.height * 2);
        else if(_lost_cnt == 40)  //40
            _res_last.size = Size2f(_res_last.size.width * 1.5, _res_last.size.height * 1.5);
        else if (_lost_cnt == 50) //18
            _res_last.size = Size2f(_res_last.size.width * 1.5, _res_last.size.height * 1.5);
        else if (_lost_cnt > 100 ) //33
            _res_last = RotatedRect();
    }
    return;
}
