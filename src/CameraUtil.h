
#include <regex>
#include <mutex>

#include <hidl/HidlSupport.h>

using android::hardware::hidl_string;

class CameraUtil{
public:
    static bool matchDeviceName(const android::hardware::hidl_string& deviceName, const android::hardware::hidl_string &providerType, std::string* deviceVersion, std::string* cameraId);

    static const int CAMERA_DEVICE_API_VERSION_3_3;
    static const int CAMERA_DEVICE_API_VERSION_3_2;
    static const int CAMERA_DEVICE_API_VERSION_1_0;
    static const std::string HAL3_3;
    static const std::string HAL3_2;
    static const std::string HAL1_0;

private:
    static const char * deviceNameRE;
};
