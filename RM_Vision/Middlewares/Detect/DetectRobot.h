#ifndef DETECTROBOT_H
#define DETECTROBOT_H

#include <QLibrary>
#include <cuda_runtime_api.h>
#include "detect_common.h"

#ifndef CUDA_CHECK
#define CUDA_CHECK(callstr)\
    {\
        cudaError_t error_code = callstr;\
        if (error_code != cudaSuccess) {\
            std::cerr << "CUDA error " << error_code << " at " << __FILE__ << ":" << __LINE__;\
            assert(0);\
        }\
    }
#endif  // CUDA_CHECK


typedef enum
{
    Car     = 0,
    Watcher = 1,
    Base    = 2,
    RedHero = 3,
    RedEngineer  = 4,
    RedInfantry1 = 5,
    RedInfantry2 = 6,
    RedInfantry3 = 7,
    RedSentry = 9,
    RedBaseArmor = 10,
    BlueHero = 11,
    BlueEngineer  = 12,
    BlueInfantry1 = 13,
    BlueInfantry2 = 14,
    BlueInfantry3 = 15,
    BlueSentry = 17,
    BlueBaseArmor = 18,
} DetectClassID_e;



typedef struct
{
    float class_id;
    float score;
    cv::Rect rect;
}DetectResult_t;

class DetectRobot
{
public:
    DetectRobot(QString& cuda_lib, int input_h, int input_w, int class_num, int max_box_count);
    ~DetectRobot();
    int initLoadEngine(std::string carEnginePath);   //add by Liying
    int getDetectResult(cv::Mat inputImage, std::vector<DetectResult_t> &detect_vector);


private:
    void doInference(IExecutionContext& context, cudaStream_t& stream, void **buffers, float* input, float* output, int batchSize);
    cv::Rect get_rect(cv::Mat &img, float bbox[]);
    void nms(std::vector<Yolo::Detection> &res, float *output, float conf_thresh, float nms_thresh);

    void* buffers[2];
    void* buffers_armor[2];

    QLibrary*           m_libCuda;
    cudaStream_t        stream_locate;
    IExecutionContext*  context_locate;
    IRuntime*           runtime_locate;
    ICudaEngine*        engine_locate;

    cudaStream_t stream_armor;
    IExecutionContext* context_armor;
    IRuntime* runtime_armor;
    ICudaEngine* engine_armor;

    bool initFlag;

    // stuff we know about the network and the input/output blobs
    int m_nInputH;
    int m_nInputW;
    int m_nClassNum;
    int m_nMaxBoxCount;
    int m_nOutputSize;  // we assume the yololayer outputs no more than MAX_OUTPUT_BBOX_COUNT boxes that conf >= 0.1
    Logger* m_Logger;
    float* input_data;
    float* output_prob;

};

#endif // DETECTROBOT_H
