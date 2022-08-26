#ifndef DETECT_COMMON_H_
#define DETECT_COMMON_H_

#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "NvInfer.h"
#include "logging.h"

using namespace nvinfer1;

namespace Yolo
{
static constexpr int CHECK_COUNT = 3;
static constexpr float IGNORE_THRESH = 0.1f;
struct YoloKernel
{
    int width;
    int height;
    float anchors[CHECK_COUNT * 2];
};

static constexpr int LOCATIONS = 4;
struct alignas(float) Detection {
    //center_x center_y w h
    float bbox[LOCATIONS];
    float conf;  // bbox_conf * cls_conf
    float class_id;
};

static constexpr int DETECT_MAX_OUTPUT_BBOX_COUNT = 500;
static constexpr int DETECT_CLASS_NUM = 27;
static constexpr int DETECT_INPUT_H = 960;
static constexpr int DETECT_INPUT_W = 960;

static constexpr int WINDMILL_MAX_OUTPUT_BBOX_COUNT = 1000;
static constexpr int WINDMILL_CLASS_NUM = 1;
static constexpr int WINDMILL_INPUT_H = 640;
static constexpr int WINDMILL_INPUT_W = 640;
}

float iou(float lbox[4], float rbox[4]);
bool cmp(const Yolo::Detection& a, const Yolo::Detection& b);
std::map<std::string, Weights> loadWeights(const std::string file);
IScaleLayer* addBatchNorm2d(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, ITensor& input, std::string lname, float eps);
ILayer* convBlock(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, ITensor& input, int outch, int ksize, int s, int g, std::string lname);
ILayer* focus(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, ITensor& input, int inch, int outch, int ksize, std::string lname);
ILayer* bottleneck(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, ITensor& input, int c1, int c2, bool shortcut, int g, float e, std::string lname);
ILayer* bottleneckCSP(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, ITensor& input, int c1, int c2, int n, bool shortcut, int g, float e, std::string lname);
ILayer* C3(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, ITensor& input, int c1, int c2, int n, bool shortcut, int g, float e, std::string lname);
ILayer* SPP(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, ITensor& input, int c1, int c2, int k1, int k2, int k3, std::string lname);
std::vector<float> getAnchors(std::map<std::string, Weights>& weightMap);
//IPluginV2Layer* addYoLoLayer(INetworkDefinition *network, std::map<std::string, Weights>& weightMap, IConvolutionLayer* det0, IConvolutionLayer* det1, IConvolutionLayer* det2);


#endif // DETECT_COMMON_H_

