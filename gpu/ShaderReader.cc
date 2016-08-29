#include "ShaderReader.h"
#ifdef TARGET_IS_ANDROID

ShaderReader::ShaderReader(JNIEnv *env, jobject assetManager, std::string apkWorkspacePath){
    this->apkWorkspacePath = apkWorkspacePath;
    assetManagerRef = (jclass) env->NewGlobalRef( assetManager );
    apkAssetManager = AAssetManager_fromJava(env, assetManagerRef);
}
ShaderReader::~ShaderReader(){

}
bool ShaderReader::getShader(std::string shader_name, std::string &shader_code) {
//    AAssetManager*  apkAssetManager = AAssetManager_fromJava(env, assetManagerRef);

    //Extract file---
    std::string assetName = shader_name;
    std::string assetPath = apkWorkspacePath + "/" + assetName;

    // Open file
    AAsset* asset = AAssetManager_open(apkAssetManager, assetName.c_str(), AASSET_MODE_STREAMING);

    char buf[BUFSIZ];
    int nb_read = 0;
    std::stringstream ss;
    if (asset != NULL)
    {
        myLOGD("Reading shader -> %s",shader_name.c_str() );
        while ((nb_read = AAsset_read(asset, buf, BUFSIZ)) > 0)
        {
            ss.write(buf,nb_read);
        }
//        myLOGD("Shader code read -> %s",ss.str().c_str());
        AAsset_close(asset);
        myLOGI("Shader extracted successfully");
        shader_code = ss.str();
        return true;
    }
    else
    {
        myLOGE("Asset not found: %s", assetName.c_str());
        shader_code = "";
        return false;
    }

}

#endif