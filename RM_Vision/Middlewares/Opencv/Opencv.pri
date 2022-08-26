#导入opencv库
DEFINES += OPENCV4_DLL
OPENCV_VERSION=411

INCLUDEPATH += /usr/include/opencv4/opencv2 \
               /usr/include/opencv4

LIBS += /usr/lib/aarch64-linux-gnu/libopencv_calib3d.so \
        /usr/lib/aarch64-linux-gnu/libopencv_core.so \
        /usr/lib/aarch64-linux-gnu/libopencv_dnn.so \
        /usr/lib/aarch64-linux-gnu/libopencv_features2d.so \
        /usr/lib/aarch64-linux-gnu/libopencv_flann.so \
        /usr/lib/aarch64-linux-gnu/libopencv_gapi.so \
        /usr/lib/aarch64-linux-gnu/libopencv_highgui.so \
        /usr/lib/aarch64-linux-gnu/libopencv_imgcodecs.so \
        /usr/lib/aarch64-linux-gnu/libopencv_imgproc.so \
        /usr/lib/aarch64-linux-gnu/libopencv_ml.so \
        /usr/lib/aarch64-linux-gnu/libopencv_objdetect.so \
        /usr/lib/aarch64-linux-gnu/libopencv_photo.so \
        /usr/lib/aarch64-linux-gnu/libopencv_stitching.so \
        /usr/lib/aarch64-linux-gnu/libopencv_video.so \
        /usr/lib/aarch64-linux-gnu/libopencv_videoio.so

