#include "Camera.h"

Camera::Camera(QObject* parent) :
    QThread(parent),
    m_bOpen(false),
    m_bCapture(false),
    m_hDevice(nullptr),
    m_i64PayLoadSize(0),
    m_i64ImageWidth(0),
    m_i64ImageHeight(0),
    m_i64ColorFilter(0),
    m_pFrameBuffer(nullptr),
    m_ui64BufferNum(0),
    m_pRaw8Image(nullptr),
    m_pCameraImage(nullptr),
    m_objParamMutex(QMutex::Recursive),
    m_objImageMutex(QMutex::Recursive)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXInitLib();
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

}

Camera::~Camera()
{
    if (m_bCapture)
    {
        m_bCapture = false;
        this->quit();
        this->wait();
    }

    // Release all resources
    RELEASE_ALLOC_MEM(m_pCameraImage);
    RELEASE_ALLOC_ARR(m_pRaw8Image);

    RELEASE_ALLOC_ARR(m_pFrameBuffer);

    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GXCloseDevice(m_hDevice);
    emStatus = GXCloseLib();
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }
}

GX_STATUS Camera::open(uint32_t index)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    emStatus = GXOpenDeviceByIndex(index, &m_hDevice);
    if (emStatus == GX_STATUS_SUCCESS)
    {
        m_bOpen = true;
        qDebug("Camera %d opened successfully", index);
    }
    else
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return emStatus;
}

GX_STATUS Camera::open(GX_OPEN_MODE_CMD open_mode, char* content, GX_ACCESS_MODE_CMD access_mode)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    GX_OPEN_PARAM  open_param;
    open_param.openMode = open_mode;
    open_param.accessMode = access_mode;
    open_param.pszContent = content;

    emStatus = GXOpenDevice(&open_param, &m_hDevice);
    if (emStatus == GX_STATUS_SUCCESS)
    {
        m_bOpen = true;
        qDebug("Camera %s opened successfully", content);
    }
    else
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return emStatus;
}

GX_STATUS Camera::close()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    if (m_bCapture)
    {
        m_bCapture = false;
//        this->quit();
//        this->wait();
    }
    if (m_bOpen)
    {
        emStatus = GXCloseDevice(m_hDevice);
        m_hDevice = nullptr;
        m_bOpen = false;
    }

    return emStatus;
}

GX_STATUS Camera::deviceInit()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    int64_t i64ImageWidth = 0;
    int64_t i64ImageHeight = 0;

    emStatus = GXGetInt(m_hDevice, GX_INT_PAYLOAD_SIZE, &m_i64PayLoadSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }

    // Get the type of Bayer conversion. whether is a color camera.
    emStatus = GXIsImplemented(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_bColorFilter);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }

    // Color image
    if(m_bColorFilter)
    {
        emStatus = GXGetEnum(m_hDevice, GX_ENUM_PIXEL_COLOR_FILTER, &m_i64ColorFilter);
        if (emStatus != GX_STATUS_SUCCESS)
        {
            CAMERA_SHOW_MESSAGA(emStatus);
            return emStatus;
        }
    }

    // Get the image width
    emStatus = GXGetInt(m_hDevice, GX_INT_WIDTH, &i64ImageWidth);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }

    // Get the image height
    emStatus = GXGetInt(m_hDevice, GX_INT_HEIGHT, &i64ImageHeight);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }

    // Release CAMERA_FRAME_BUFFER array
    RELEASE_ALLOC_ARR(m_pFrameBuffer);

    m_pFrameBuffer = new CAMERA_FRAME_BUFFER;
    m_pFrameBuffer->pImgBuf = malloc((size_t)m_i64PayLoadSize);

    // If width or height is changed, realloc image buffer
    if (i64ImageWidth != m_i64ImageWidth || i64ImageHeight != m_i64ImageHeight)
    {
        m_i64ImageWidth = i64ImageWidth;
        m_i64ImageHeight = i64ImageHeight;

        RELEASE_ALLOC_ARR(m_pRaw8Image);
        RELEASE_ALLOC_MEM(m_pCameraImage);

        try
        {
            // Allocate raw8 frame buffer for DxRaw16toRaw8
            m_pRaw8Image = new unsigned char[m_i64ImageWidth * m_i64ImageHeight];


#ifdef OPENCV4_DLL
            // Allocate three QImage buffer for deque acquisition
            m_pCameraImage = new cv::Mat(m_i64ImageHeight, m_i64ImageWidth, CV_8UC3);
#else
            // Allocate three QImage buffer for deque acquisition
            m_pCameraImage = new QImage(m_i64ImageWidth, m_i64ImageHeight, QImage::Format_RGB888);
#endif

        }
        catch (std::bad_alloc& e)
        {
           qDebug("Error: Start Acquisition Failed : Allocate image resources failed! ");
           return false;
        }
    }

    return emStatus;
}



GX_STATUS Camera::updateDeviceList(GX_DEVICE_BASE_INFO* base_info, uint32_t* device_num)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    // Enumerate Devcie List
    emStatus = GXUpdateDeviceList(device_num, ENUMERATE_TIME_OUT);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    if (*device_num > 0)
    {
        // Alloc resourses for device baseinfo
        try
        {
            base_info = new GX_DEVICE_BASE_INFO[*device_num];
        }
        catch (std::bad_alloc &e)
        {
            qDebug("Allocate memory error: Cannot allocate memory, please exit this app!");
            RELEASE_ALLOC_MEM(base_info);
            return GX_STATUS_ERROR;
        }
        // Set size of function "GXGetAllDeviceBaseInfo"
        size_t nSize = *device_num * sizeof(GX_DEVICE_BASE_INFO);

        // Get all device baseinfo
        emStatus = GXGetAllDeviceBaseInfo(base_info, &nSize);
        if (emStatus != GX_STATUS_SUCCESS)
        {
            RELEASE_ALLOC_ARR(base_info);
            CAMERA_SHOW_MESSAGA(emStatus);
            return emStatus;
        }
    }
    return emStatus;
}

void Camera::startCapture()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    if (m_bCapture)
    {
        return;
    }

    // 发开始采集命令
    emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_START);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return;
    }

    emStatus = deviceInit();
    m_bCapture = true;
    this->start(QThread::NormalPriority);
}

void Camera::stopCapture()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    if (!m_bCapture)
    {
        return;
    }

    emStatus = GXSendCommand(m_hDevice, GX_COMMAND_ACQUISITION_STOP);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return;
    }

    m_bCapture = false;
}

GX_DEV_HANDLE Camera::getDeviceHandle()
{
    return m_hDevice;
}

GX_STATUS Camera::setWhiteBalance(double red_ratio, double green_ratio, double blue_ratio)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    emStatus = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_RED);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }
    emStatus = GXSetFloat(m_hDevice, GX_FLOAT_BALANCE_RATIO, red_ratio);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }

    emStatus = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_GREEN);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }
    emStatus = GXSetFloat(m_hDevice, GX_FLOAT_BALANCE_RATIO, green_ratio);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }

    emStatus = GXSetEnum(m_hDevice, GX_ENUM_BALANCE_RATIO_SELECTOR, GX_BALANCE_RATIO_SELECTOR_BLUE);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }
    emStatus = GXSetFloat(m_hDevice, GX_FLOAT_BALANCE_RATIO, blue_ratio);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
        return emStatus;
    }

    return emStatus;
}

GX_STATUS Camera::setExposureTime(double exposure_time)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    emStatus = GXSetFloat(m_hDevice, GX_FLOAT_EXPOSURE_TIME, exposure_time);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return emStatus;
}

GX_STATUS Camera::setGain(double gain)
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;

    emStatus = GXSetFloat(m_hDevice, GX_FLOAT_GAIN, gain);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return emStatus;
}

QString Camera::getVendorName()
{
    // Get Vendor Name
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    char arrNameString[128];
    size_t nSize = sizeof(arrNameString);
    emStatus = GXGetString(m_hDevice, GX_STRING_DEVICE_VENDOR_NAME, arrNameString, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return QString(arrNameString);
}

QString Camera::getModelName()
{
    // Get Model Name
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    char arrNameString[128];
    size_t nSize = sizeof(arrNameString);
    emStatus = GXGetString(m_hDevice, GX_STRING_DEVICE_MODEL_NAME, arrNameString, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return QString(arrNameString);
}

QString Camera::getSerialNumber()
{
    // Get Serial Number
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    char arrNameString[128];
    size_t nSize = sizeof(arrNameString);
    emStatus = GXGetString(m_hDevice, GX_STRING_DEVICE_SERIAL_NUMBER, arrNameString, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return QString(arrNameString);
}

QString Camera::getDeviceVersion()
{
    // Get Device Version
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    char arrNameString[128];
    size_t nSize = sizeof(arrNameString);
    emStatus = GXGetString(m_hDevice, GX_STRING_DEVICE_VERSION, arrNameString, &nSize);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    return QString(arrNameString);
}

bool Camera::isOpen()
{
    return m_bOpen;
}

bool Camera::isCapture()
{
    return m_bCapture;
}

CameraImage* Camera::getFrame()
{
    QMutexLocker locker(&m_objImageMutex);
    return m_pCameraImage;
}

double Camera::getFps()
{
    m_objFps.UpdateFps();
    double dAcqFrameRate = m_objFps.GetFps();

    return dAcqFrameRate;
}

uint32_t Camera::getFrameCount()
{
    return m_nFrameCount;
}

//图像采集
void Camera::run()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    bool bProcStatus = false;
    m_nFrameCount = 0;

    while (m_bCapture)
    {

        emStatus = GXGetImage(m_hDevice, m_pFrameBuffer, 500);
        if (emStatus != GX_STATUS_SUCCESS)
        {
            CAMERA_SHOW_MESSAGA(emStatus);
            continue;
        }


#ifdef OPENCV4_DLL
        unsigned char* pImageProcess = m_pCameraImage->data;
#else
        // Assign the address of the first pixel of the QImage to a temporary variable for image processing
        unsigned char* pImageProcess = m_pCameraImage->bits();
#endif

        // Image processing, Raw to RGB24 and image improvment, if process failed put buffer back to buffer deque
        bProcStatus = ImageProcess(m_pFrameBuffer, pImageProcess);

        // Get acquisition frame rate
//        for (uint32_t i = 0; i < ui32FrameNum; i++)
        {
            m_nFrameCount++;
            m_objFps.IncreaseFrameNum();
        }
    }

    // Acquisition fps reset
    m_objFps.Reset();

    close();
}


bool Camera::ImageProcess(const CAMERA_FRAME_BUFFER* frame_buffer, unsigned char* pixel_data)
{
    QMutexLocker locker(&m_objImageMutex);
    VxInt32 emDxStatus = DX_OK;

    // Convert RAW8 or RAW16 image to RGB24 image
    switch (frame_buffer->nPixelFormat)
    {
        case GX_PIXEL_FORMAT_MONO8:
        case GX_PIXEL_FORMAT_MONO8_SIGNED:
        case GX_PIXEL_FORMAT_BAYER_GR8:
        case GX_PIXEL_FORMAT_BAYER_RG8:
        case GX_PIXEL_FORMAT_BAYER_GB8:
        case GX_PIXEL_FORMAT_BAYER_BG8:
        {
            // Convert to the RGB
            emDxStatus = DxRaw8toRGB24((unsigned char*)frame_buffer->pImgBuf, pixel_data, m_i64ImageWidth, m_i64ImageHeight,
                              RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(m_i64ColorFilter), false);
            if (emDxStatus != DX_OK)
            {
                return false;
            }
            break;
        }
        case GX_PIXEL_FORMAT_MONO10:
        case GX_PIXEL_FORMAT_BAYER_GR10:
        case GX_PIXEL_FORMAT_BAYER_RG10:
        case GX_PIXEL_FORMAT_BAYER_GB10:
        case GX_PIXEL_FORMAT_BAYER_BG10:
        {
            // Convert to the Raw8 image
            emDxStatus = DxRaw16toRaw8((unsigned char*)frame_buffer->pImgBuf, m_pRaw8Image, m_i64ImageWidth, m_i64ImageHeight, DX_BIT_2_9);
            if (emDxStatus != DX_OK)
            {
                return false;
            }
            // Convert to the RGB24 image
            emDxStatus = DxRaw8toRGB24((unsigned char*)m_pRaw8Image, pixel_data, m_i64ImageWidth, m_i64ImageHeight,
                              RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(m_i64ColorFilter), false);
            if (emDxStatus != DX_OK)
            {
                return false;
            }
            break;
        }
        case GX_PIXEL_FORMAT_MONO12:
        case GX_PIXEL_FORMAT_BAYER_GR12:
        case GX_PIXEL_FORMAT_BAYER_RG12:
        case GX_PIXEL_FORMAT_BAYER_GB12:
        case GX_PIXEL_FORMAT_BAYER_BG12:
        {
            // Convert to the Raw8 image
            emDxStatus = DxRaw16toRaw8((unsigned char*)frame_buffer->pImgBuf, m_pRaw8Image, m_i64ImageWidth, m_i64ImageHeight, DX_BIT_4_11);
            if (emDxStatus != DX_OK)
            {
                return false;
            }
            // Convert to the RGB24 image
            emDxStatus = DxRaw8toRGB24((unsigned char*)m_pRaw8Image, pixel_data, m_i64ImageWidth, m_i64ImageHeight,
                              RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(m_i64ColorFilter), false);
            if (emDxStatus != DX_OK)
            {
                return false;
            }
            break;
        }
        default:
        {
            qDebug("Unknown Pixel Format %d", frame_buffer->nPixelFormat);
            // Enter this branch when pixel format not support
            return false;
        }
    }

//    emDxStatus = DxRaw8toRGB24Ex((unsigned char*)frame_buffer->pImgBuf, pixel_data, m_i64ImageWidth, m_i64ImageHeight,
//                      RAW2RGB_NEIGHBOUR, DX_PIXEL_COLOR_FILTER(m_i64ColorFilter), false, DX_ORDER_RGB);

    // Image improvment params will changed in other thread, must being locked
//    QMutexLocker locker(&m_objParamMutex);

//    int64_t i64ColorCorrection = m_bColorCorrection ? m_i64ColorCorrection : 0;
//    unsigned char* pGammaLut = m_bGammaRegulation ? m_pGammaLut : NULL;
//    unsigned char* pContrastLut = m_bContrastRegulation ? m_pContrastLut : NULL;

//    if (i64ColorCorrection != 0 || pGammaLut != NULL || pContrastLut != NULL)
//    {
//        emDxStatus = DxImageImprovment(pImageProcess, pImageProcess, m_i64ImageWidth, m_i64ImageHeight, i64ColorCorrection, pContrastLut, pGammaLut);
//        if (emDxStatus != DX_OK)
//        {
//            emit SigImageProcError(emDxStatus);
//            return PROC_FAIL;
//        }
//    }
    return true;
}

void Camera::setAcquisitionBufferNum()
{
    GX_STATUS emStatus = GX_STATUS_SUCCESS;
    uint64_t ui64BufferNum = 0;

    // Calculate a reasonable number of Buffers for different payload size
    // Small ROI and high frame rate will requires more acquisition Buffer
    const size_t MAX_MEMORY_SIZE = 8 * 1024 * 1024; // The maximum number of memory bytes available for allocating frame Buffer
    const size_t MIN_BUFFER_NUM  = 5;               // Minimum frame Buffer number
    const size_t MAX_BUFFER_NUM  = 450;             // Maximum frame Buffer number
    ui64BufferNum = MAX_MEMORY_SIZE / m_i64PayLoadSize;
    ui64BufferNum = (ui64BufferNum <= MIN_BUFFER_NUM) ? MIN_BUFFER_NUM : ui64BufferNum;
    ui64BufferNum = (ui64BufferNum >= MAX_BUFFER_NUM) ? MAX_BUFFER_NUM : ui64BufferNum;

    emStatus = GXSetAcqusitionBufferNumber(m_hDevice, ui64BufferNum);
    if (emStatus != GX_STATUS_SUCCESS)
    {
        CAMERA_SHOW_MESSAGA(emStatus);
    }

    m_ui64BufferNum = ui64BufferNum;
}

