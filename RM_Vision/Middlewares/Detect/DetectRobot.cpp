#include "DetectRobot.h"
#include "Common.h"

#define USE_FP16  // set USE_INT8 or USE_FP16 or USE_FP32
#define DEVICE 0  // GPU id
#define NMS_THRESH 0.4
#define CONF_THRESH 0.5
#define BATCH_SIZE 1

// stuff we know about the network and the input/output blobs
const char* INPUT_BLOB_NAME = "data";
const char* OUTPUT_BLOB_NAME = "prob";


static inline cv::Mat preprocess_img(cv::Mat& img, int input_w, int input_h) {
    int w, h, x, y;
    float r_w = input_w / (img.cols*1.0);
    float r_h = input_h / (img.rows*1.0);
    if (r_h > r_w) {
        w = input_w;
        h = r_w * img.rows;
        x = 0;
        y = (input_h - h) / 2;
    } else {
        w = r_h * img.cols;
        h = input_h;
        x = (input_w - w) / 2;
        y = 0;
    }
    cv::Mat re(h, w, CV_8UC3);
    cv::resize(img, re, re.size(), 0, 0, cv::INTER_LINEAR);
    cv::Mat out(input_h, input_w, CV_8UC3, cv::Scalar(128, 128, 128));
    re.copyTo(out(cv::Rect(x, y, re.cols, re.rows)));
    return out;
}

DetectRobot::DetectRobot(QString& cuda_lib, int input_h, int input_w, int class_num, int max_box_count) :
    m_nInputH(input_h),
    m_nInputW(input_w),
    m_nClassNum(class_num),
    m_nMaxBoxCount(max_box_count),
    m_nOutputSize(max_box_count * sizeof(Yolo::Detection) / sizeof(float) + 1),
    m_Logger(new Logger),
    input_data(new float[BATCH_SIZE * 3 * m_nInputH * m_nInputW]),
    output_prob(new float[BATCH_SIZE * m_nOutputSize])
{
    if (cuda_lib.isEmpty())
        qDebug("cuda_lib path empty!");

    m_libCuda = new QLibrary(cuda_lib);
    m_libCuda->load();
    if (!m_libCuda->isLoaded())
        qDebug("Load %s failed!", qPrintable(cuda_lib));


    initFlag = false;
    //CUDA_CHECK(cudaStreamCreate(&stream_locate));
    context_locate = nullptr;
    runtime_locate = nullptr;
    engine_locate = nullptr;

    CUDA_CHECK(cudaStreamCreate(&stream_armor));
    context_armor = nullptr;
    runtime_armor = nullptr;
    engine_armor = nullptr;
}

DetectRobot::~DetectRobot()
{
    cudaStreamDestroy(stream_locate);
    CUDA_CHECK(cudaFree(buffers[0]));
    CUDA_CHECK(cudaFree(buffers[1]));
    context_locate->destroy();
    engine_locate->destroy();
    runtime_locate->destroy();

    //cudaStreamDestroy(stream_armor);
    //CUDA_CHECK(cudaFree(buffers_armor[0]));
    //CUDA_CHECK(cudaFree(buffers_armor[1]));
    //context_armor->destroy();
    //engine_armor->destroy();
    //runtime_armor->destroy();

    initFlag = false;
    RELEASE_ALLOC_MEM(m_libCuda);
    RELEASE_ALLOC_MEM(m_Logger);
    RELEASE_ALLOC_ARR(input_data);
    RELEASE_ALLOC_ARR(output_prob);
}

int DetectRobot::initLoadEngine(std::string carEnginePath)
{
    std::string engine_name = carEnginePath;
    std::ifstream file(engine_name, std::ios::binary);
    if (!file.good())
    {
        std::cerr << "read " << engine_name << " error!" << std::endl;
        return -1;
    }
    char *trtModelStream = nullptr;
    size_t size = 0;
    file.seekg(0, file.end);
    size = file.tellg();
    file.seekg(0, file.beg);
    trtModelStream = new char[size];
    assert(trtModelStream);
    file.read(trtModelStream, size);
    file.close();

    // prepare input data ---------------------------
    runtime_locate = createInferRuntime(*m_Logger);
    assert(runtime_locate != nullptr);
    engine_locate = runtime_locate->deserializeCudaEngine(trtModelStream, size);
    assert(engine_locate != nullptr);
    context_locate = engine_locate->createExecutionContext();
    assert(context_locate != nullptr);
    delete[] trtModelStream;
    assert(engine_locate->getNbBindings() == 2);
    //void* buffers[2];
    // In order to bind the buffers, we need to know the names of the input and output tensors.
    // Note that indices are guaranteed to be less than IEngine::getNbBindings()
    const int inputIndex = engine_locate->getBindingIndex(INPUT_BLOB_NAME);
    const int outputIndex = engine_locate->getBindingIndex(OUTPUT_BLOB_NAME);
    assert(inputIndex == 0);
    assert(outputIndex == 1);
    // Create GPU buffers on device
    CUDA_CHECK(cudaMalloc(&buffers[inputIndex], BATCH_SIZE * 3 * m_nInputH * m_nInputW * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&buffers[outputIndex], BATCH_SIZE * m_nOutputSize * sizeof(float)));
    CUDA_CHECK(cudaStreamCreate(&stream_locate));
    std::cout<<"load " << engine_name << " succeed!"<<std::endl;
    initFlag = true;
    return 1;
}

int DetectRobot::getDetectResult(cv::Mat inputImage, std::vector<DetectResult_t> &detect_vector)
{
    if(!initFlag)
    {
        std::cerr << "please init load engine first ." << std::endl;
        return -1;
    }

    detect_vector.clear();
    int fcount = 1;

    cv::Mat img = inputImage.clone();
    if (img.empty())
    {
        std::cerr << "input image is empty ." << std::endl;
        return -1;
    }


    cv::Mat pr_img = preprocess_img(img, m_nInputW, m_nInputH); // letterbox BGR to RGB

    int i = 0;
    for (int row = 0; row < m_nInputH; ++row)
    {
        uchar* uc_pixel = pr_img.data + row * pr_img.step;
        for (int col = 0; col < m_nInputW; ++col)
        {
            input_data[i] = (float)uc_pixel[2] / 255.0;
            input_data[i + m_nInputH * m_nInputW] = (float)uc_pixel[1] / 255.0;
            input_data[i + 2 * m_nInputH * m_nInputW] = (float)uc_pixel[0] / 255.0;
            uc_pixel += 3;
            ++i;
        }
    }

    // Run inference
    doInference(*context_locate, stream_locate, buffers, input_data, output_prob, BATCH_SIZE);
    std::vector<std::vector<Yolo::Detection>> batch_res(fcount);
    for (int b = 0; b < fcount; b++)
    {
        auto& res = batch_res[b];
        nms(res, &output_prob[b * m_nOutputSize], CONF_THRESH, NMS_THRESH);
    }


    for (int b = 0; b < fcount; b++)
    {

        auto& res = batch_res[b];
        for (size_t j = 0; j < res.size(); j++)
        {
            DetectResult_t detect_res;
            detect_res.class_id = (int)res[j].class_id;
            detect_res.score = res[j].conf;
            detect_res.rect = get_rect(img, res[j].bbox);

//            detect_vector.push_back(detect_res);
            detect_vector.emplace_back(detect_res); //fast
        }
    }

    return 1;
}

void DetectRobot::doInference(IExecutionContext& context, cudaStream_t& stream, void **buffers, float* input, float* output, int batchSize) {
    // DMA input batch data to device, infer on the batch asynchronously, and DMA output back to host
    //std::cout<<"yes1"<<std::endl;
    CUDA_CHECK(cudaMemcpyAsync(buffers[0], input, batchSize * 3 * m_nInputH * m_nInputW * sizeof(float), cudaMemcpyHostToDevice, stream));
   // std::cout<<"yes2"<<std::endl;

    context.enqueue(batchSize, buffers, stream, nullptr);
    // std::cout<<"yes3"<<std::endl;

    CUDA_CHECK(cudaMemcpyAsync(output, buffers[1], batchSize * m_nOutputSize * sizeof(float), cudaMemcpyDeviceToHost, stream));
    cudaStreamSynchronize(stream);
}

cv::Rect DetectRobot::get_rect(cv::Mat& img, float bbox[4])
{
    int l, r, t, b;
    float r_w = m_nInputW / (img.cols * 1.0);
    float r_h = m_nInputH / (img.rows * 1.0);
    if (r_h > r_w) {
        l = bbox[0] - bbox[2] / 2.f;
        r = bbox[0] + bbox[2] / 2.f;
        t = bbox[1] - bbox[3] / 2.f - (m_nInputH - r_w * img.rows) / 2;
        b = bbox[1] + bbox[3] / 2.f - (m_nInputH - r_w * img.rows) / 2;
        l = l / r_w;
        r = r / r_w;
        t = t / r_w;
        b = b / r_w;
    } else {
        l = bbox[0] - bbox[2] / 2.f - (m_nInputW - r_h * img.cols) / 2;
        r = bbox[0] + bbox[2] / 2.f - (m_nInputW - r_h * img.cols) / 2;
        t = bbox[1] - bbox[3] / 2.f;
        b = bbox[1] + bbox[3] / 2.f;
        l = l / r_h;
        r = r / r_h;
        t = t / r_h;
        b = b / r_h;
    }
    return cv::Rect(l, t, r - l, b - t);
}


void DetectRobot::nms(std::vector<Yolo::Detection>& res, float *output, float conf_thresh, float nms_thresh)
{
    int det_size = sizeof(Yolo::Detection) / sizeof(float);
    std::map<float, std::vector<Yolo::Detection>> m;
    for (int i = 0; i < output[0] && i < m_nMaxBoxCount; i++) {
        if (output[1 + det_size * i + 4] <= conf_thresh) continue;
        Yolo::Detection det;
        memcpy(&det, &output[1 + det_size * i], det_size * sizeof(float));
        if (m.count(det.class_id) == 0) m.emplace(det.class_id, std::vector<Yolo::Detection>());
        m[det.class_id].push_back(det);
    }
    for (auto it = m.begin(); it != m.end(); it++) {
        //std::cout << it->second[0].class_id << " --- " << std::endl;
        auto& dets = it->second;
        std::sort(dets.begin(), dets.end(), cmp);
        for (size_t m = 0; m < dets.size(); ++m) {
            auto& item = dets[m];
            res.push_back(item);
            for (size_t n = m + 1; n < dets.size(); ++n) {
                if (iou(item.bbox, dets[n].bbox) > nms_thresh) {
                    dets.erase(dets.begin() + n);
                    --n;
                }
            }
        }
    }
}


