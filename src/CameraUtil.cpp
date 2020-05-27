#include <utils/String8.h>
#include "CameraUtil.h"

using android::hardware::hidl_string;

const char * CameraUtil::deviceNameRE = "device@([0-9]+\\.[0-9]+)/%s/(.+)";

const int CameraUtil::CAMERA_DEVICE_API_VERSION_3_3 = 0x303;
const int CameraUtil::CAMERA_DEVICE_API_VERSION_3_2 = 0x302;
const int CameraUtil::CAMERA_DEVICE_API_VERSION_1_0 = 0x100;
const std::string CameraUtil::HAL3_3 = "3.3";
const std::string CameraUtil::HAL3_2 = "3.2";
const std::string CameraUtil::HAL1_0 = "1.0";

bool CameraUtil::matchDeviceName(const hidl_string& deviceName, const hidl_string &providerType, std::string* deviceVersion, std::string* cameraId){
    android::String8 pattern;

    pattern.appendFormat(deviceNameRE, providerType.c_str());
    std::regex e(pattern.string());
    std::string deviceNameStd(deviceName.c_str());
    std::smatch sm;
    if(std::regex_match(deviceNameStd, sm, e)){
        if(deviceVersion != nullptr){
            *deviceVersion = sm[1];
        }
        if(cameraId != nullptr){
            *cameraId = sm[2];
        }
        return true;
    }
    return false;
}
