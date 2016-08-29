GPU_LOCAL_PATH := $(call my-dir)
LOCAL_PATH := $(GPU_LOCAL_PATH)

include $(CLEAR_VARS)
include $(OPENCV_PATH)/OpenCV.mk
LOCAL_MODULE            := gpu
LOCAL_SRC_FILES         := base_gpgpu.cpp \
                           gl_apis.cpp \
                           klt_gpu.cpp \
                           ShaderReader.cc
LOCAL_CPP_EXTENSION     := .cxx .cpp .cc
LOCAL_C_INCLUDES        += $(GPU_LOCAL_PATH)..\common
LOCAL_EXPORT_C_INCLUDES += $(GPU_LOCAL_PATH)
LOCAL_C_INCLUDES        += $(GLM_PATH)
#LOCAL_STATIC_LIBRARIES  += gl_stuff camera

include $(BUILD_STATIC_LIBRARY)
