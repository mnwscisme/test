/*******************************************************************************************************************
Copyright 2017 Dajiang Innovations Technology Co., Ltd (DJI)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
documentation files(the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and 
to permit persons to whom the Software is furnished to do so, subject to the following conditions : 

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*******************************************************************************************************************/

#include "AngleSolver.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

void RectPnPSolver::solvePnP4Points(const std::vector<cv::Point2f> & points2d, cv::Mat & rot, cv::Mat & trans, double & dist){
    if(width_target < 10e-5 || height_target < 10e-5){
        rot = cv::Mat::eye(3,3,CV_64FC1);
        trans = cv::Mat::zeros(3,1,CV_64FC1);
    }

    std::vector<cv::Point3f> point3d;
    double half_x = width_target / 2.0;
    double half_y = height_target / 2.0;

    point3d.push_back(Point3f( half_x, half_y, 0));
    point3d.push_back(Point3f( half_x,-half_y, 0));
    point3d.push_back(Point3f(-half_x,-half_y, 0));
    point3d.push_back(Point3f(-half_x, half_y, 0));

//    point3d.push_back(Point3f(-half_x, -half_y, 0));
//    point3d.push_back(Point3f(half_x, -half_y, 0));
//    point3d.push_back(Point3f(half_x, half_y, 0));
//    point3d.push_back(Point3f(-half_x, half_y, 0));

    cv::Mat r;
    cv::solvePnP(point3d, points2d, cam_matrix, distortion_coeff, r, trans);
    Rodrigues(r, rot);
    //dist = trans.at<double>(0, 2);
    dist = sqrt(trans.at<double>(0, 0) * trans.at<double>(0, 0) + trans.at<double>(0, 1) * trans.at<double>(0, 1)
            + trans.at<double>(0, 2) * trans.at<double>(0, 2));
}


bool AngleSolver::getAngle(const cv::RotatedRect & rect, double & pitch, double & yaw, double & distance){
    if (rect.size.height < 1)
        return false;

    vector<Point2f> target2d;
    getTarget2dPoinstion(rect, target2d);

    Mat rvec(3, 1, cv::DataType<double>::type);
    Mat tvec(3, 1, cv::DataType<double>::type);
    solvePnP4Points(target2d, rvec, tvec, distance);

    double x_pos = tvec.at<double>(0, 0);
    double y_pos = tvec.at<double>(1, 0);
    double z_pos = tvec.at<double>(2, 0);

    double tan_pitch = y_pos / sqrt(x_pos*x_pos + z_pos * z_pos);
    double tan_yaw = x_pos / z_pos;
    pitch = -atan(tan_pitch) * 180 / CV_PI;
    yaw = atan(tan_yaw) * 180 / CV_PI;

    return true;
}

bool AngleSolver::getAngle(const cv::Point2f points[4], double & pitch, double & yaw, double & distance){

    vector<Point2f> target2d;
    for (int pnp = 0; pnp<4; pnp++)
    {
        target2d.emplace_back(points[pnp]);
    }

    Mat rvec(3, 1, cv::DataType<double>::type);
    Mat tvec(3, 1, cv::DataType<double>::type);
    solvePnP4Points(target2d, rvec, tvec, distance);

    double x_pos = tvec.at<double>(0, 0);
    double y_pos = tvec.at<double>(1, 0);
    double z_pos = tvec.at<double>(2, 0);

    double tan_pitch = y_pos / sqrt(x_pos*x_pos + z_pos * z_pos);
    double tan_yaw = x_pos / z_pos;
    pitch = -atan(tan_pitch) * 180 / CV_PI;
    yaw = atan(tan_yaw) * 180 / CV_PI;

    return true;
}

void AngleSolver::getTarget2dPoinstion(const cv::RotatedRect & rect, vector<Point2f> & target2d, const cv::Point2f & offset){
    Point2f vertices[4];
    rect.points(vertices);
    Point2f lu, ld, ru, rd;
    sort(vertices, vertices + 4, [](const Point2f & p1, const Point2f & p2) { return p1.x < p2.x; });
    if (vertices[0].y < vertices[1].y){
        lu = vertices[0];
        ld = vertices[1];
    }
    else{
        lu = vertices[1];
        ld = vertices[0];
    }
    if (vertices[2].y < vertices[3].y)	{
        ru = vertices[2];
        rd = vertices[3];
    }
    else {
        ru = vertices[3];
        rd = vertices[2];
    }

    target2d.clear();
    target2d.push_back(ru + offset);
    target2d.push_back(rd + offset);
    target2d.push_back(ld + offset);
    target2d.push_back(lu + offset);
}


