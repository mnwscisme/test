#导入CUDA库
DEFINES += CUDA_DLL
CUDA_VERSION=10.2.89
CUDNN_VERSION=8.0
TENSORRT_VERSION=7.1.3

INCLUDEPATH += /usr/local/cuda/include

LIBS += /usr/local/cuda/lib64/libcudart.so \
        /usr/local/cuda/lib64/libcufft.so \
        /usr/lib/aarch64-linux-gnu/libnvinfer.so
