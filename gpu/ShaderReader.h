#ifndef SHADERREADERH
#define SHADERREADERH

#ifdef TARGET_IS_ANDROID

#include <string>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <sstream>
#include "../common/mylog.h"

class ShaderReader{
public:
    ShaderReader(JNIEnv *env, jobject assetManager, std::string apkWorkspacePath);
    ~ShaderReader();
    bool getShader(std::string shader_name, std::string &shader_code);
private:
    jobject assetManagerRef;
    std::string apkWorkspacePath;
    AAssetManager*  apkAssetManager;
};


#endif
#endif