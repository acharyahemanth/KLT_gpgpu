## nethra android makefile

KLT_GPGPU_LOCAL_PATH := $(call my-dir)
LOCAL_PATH := $(KLT_GPGPU_LOCAL_PATH)

include $(CLEAR_VARS)
include $(OPENCV_PATH)/OpenCV.mk
LOCAL_MODULE            := klt_gpgpu
LOCAL_STATIC_LIBRARIES  += gpu
include $(BUILD_STATIC_LIBRARY)

include $(KLT_GPGPU_LOCAL_PATH)/gpu/Android.mk