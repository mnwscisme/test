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

#pragma once
#include "opencv2/core/core.hpp"

/**
 * @brief The RectPnPSolver class
 * solve the pnp problem of rectangle target.
 */
class RectPnPSolver {
public:
    RectPnPSolver(const cv::Mat & camera_matrix, const cv::Mat & dist_coeff, double target_width = 0, double target_height = 0){
        camera_matrix.copyTo(cam_matrix);
        dist_coeff.copyTo(distortion_coeff);

        width_target = target_width;
        height_target = target_height;
    }

    void setTargetSize(double width, double height){
        width_target = width;
        height_target = height;
    }

    void setCameraParam(const cv::Mat & camera_matrix, const cv::Mat & dist_coeff){
        camera_matrix.copyTo(cam_matrix);
        dist_coeff.copyTo(distortion_coeff);
    }

    /**
     * @brief solvePnP
     * @param points2d image points set with following order : left_up, right_up, left_down, right_down
     * @param rot rotation between camera and target center
     * @param trans tanslation between camera and target center
     */
    void solvePnP4Points(const std::vector<cv::Point2f> & points2d, cv::Mat & rot, cv::Mat & trans, double & dist);

public:
    cv::Mat cam_matrix;
    cv::Mat distortion_coeff;
    double width_target;
    double height_target;
};

class AngleSolver : public RectPnPSolver {
public:
    AngleSolver(const cv::Mat & camera_matrix, const cv::Mat & dist_coeff,
                double target_width = 0, double target_height = 0)
        : RectPnPSolver(camera_matrix, dist_coeff, target_width, target_height){

    }

    /**
     * @brief getAngle
     * @param rect input target rectangle

     * @param angle_x output angle on x axis
     * @param angle_y output angle on y axis
     * @param offset input rectangle offset, default (0,0)
     * @return
     */
    bool getAngle(const cv::RotatedRect & rect, double & pitch, double & yaw, double & distance);
    bool getAngle(const cv::Point2f points[4], double & pitch, double & yaw, double & distance);

    void getTarget2dPoinstion(const cv::RotatedRect & rect, std::vector<cv::Point2f> & target2d, const cv::Point2f & offset = cv::Point2f(0, 0));
};

