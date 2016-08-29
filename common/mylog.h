#   include <android/log.h>
#	define LOG_TAG	"klt_gpgpu_native"
#   define  myLOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#   define  myLOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#   define  myLOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG,__VA_ARGS__)
#   define  myLOGI(...)  __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__)
#   define  myLOGW(...)  __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__)
#   define  myLOGF(...)  __android_log_print(ANDROID_LOG_FATAL   , LOG_TAG,__VA_ARGS__)
