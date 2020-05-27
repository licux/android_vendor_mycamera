#include <utility>
#include <typeinfo>
#include <unistd.h>
#include <android/log.h>
// #include <camera/NdkCameraManager.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/Log.h>
#include <utils/StrongPointer.h>
#include <utils/String8.h>
#include <grallocusage/GrallocUsageConversion.h>
#include <ui/GraphicBuffer.h>


#include <android/hardware/camera/provider/2.4/ICameraProvider.h>
#include <android/hardware/camera/device/3.2/ICameraDevice.h>
#include <android/hardware/camera/device/3.2/ICameraDeviceCallback.h>
#include <android/hardware/camera/device/3.3/ICameraDeviceSession.h>
#include <MyCameraProvider.h>
#include <MyCameraDevice.h>
#include <MyCameraDeviceCallback.h>
#include <MyCameraDeviceSession.h>

#include <CameraMetadata.h>
#include "CameraUtil.h"

#define LOGTAG "MyCamera"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOGTAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOGTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__))

using ::android::GraphicBuffer;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::sp;
using ::android::hardware::camera::device::V3_2::BufferStatus;
using ::android::hardware::camera::device::V3_2::BufferCache;
using ::android::hardware::camera::device::V3_2::CaptureRequest;
using ::android::hardware::camera::device::V3_2::StreamBuffer;
using ::android::hardware::camera::common::V1_0::Status;
using ::android::hardware::camera::device::V3_2::Stream;

int main(){
    // ACameraManager* cameraManager = ACameraManager_create();
    // if(cameraManager == NULL){
    //     LOGD("cameraManager is null");
    // }

    // ACameraIdList* cameraId = NULL;
    // ACameraManager_getCameraIdList(cameraManager, &cameraId);
    // LOGD("CameraIdList size:%d", cameraId->numCameras);

    sp<android::hardware::camera::provider::V2_4::ICameraProvider> provider =
        android::hardware::camera::provider::V2_4::ICameraProvider::getService("mycamera");
    sp<android::hardware::camera::device::V3_2::ICameraDevice> cameraDevice;
    sp<android::hardware::camera::device::V3_2::ICameraDeviceSession> cameraSession;
    android::hardware::camera::device::V3_2::CameraMetadata metadata;
    // camera_metadata_t **metadata;
    // sp<android::hardware::camera::device::V3_2::HalStreamConfiguration> halStreamConfig;
    android::hardware::camera::device::V3_2::HalStreamConfiguration halStreamConfig;

    if(provider == NULL){
        LOGD("provider is null");
    }

    hidl_vec<hidl_string> cameraDeviceNames;
    provider->getCameraIdList(
        [&](auto status, const auto& idList) {
            LOGI("getCameraIdList returns status:%d", (int)status);
            for (size_t i = 0; i < idList.size(); i++) {
                LOGI("Camera Id[%zu] is %s", i, idList[i].c_str());
            }
            cameraDeviceNames = idList;
        }
    );

    std::string version, cameraId;
    if(CameraUtil::matchDeviceName(cameraDeviceNames[0], "mycamera", &version, &cameraId)){
        LOGI("version:%s, cameraId:%s", version.c_str(), cameraId.c_str());
        if(version.compare(CameraUtil::HAL3_3) == 0 || version.compare(CameraUtil::HAL3_2) == 0){
        // if(0){
            provider->getCameraDeviceInterface_V3_x(cameraDeviceNames[0], 
                [&](auto status, const auto& device3_x){
                    LOGI("getCameraDeviceInterface_V3_x returns status:%d", (int)status);
                    cameraDevice = device3_x;
            });
        }else if(0){//version.compare(CameraUtil::HAL1_0) == 0){
            LOGE("Device 1.0 is not supported");
            // provider->getCameraDeviceInterface_V1_x(cameraDeviceNames[0], 
            //     [&](auto status, const auto& device1_0){
            //         LOGI("getCameraDeviceInterface_V1_x returns status:%d", (int)status);
            // });
        }else{
            LOGE("Device did not match");
        }
    }

    // sp<android::hardware::camera::device::V3_2::ICameraDeviceCallback> callback;
    sp<android::hardware::camera::device::V3_2::ICameraDeviceCallback> callback = new vendor::masaki::hardware::camera::device::V3_3::implementation::MyCameraDeviceCallback();
    LOGI("Call CameraDevice::open");
    cameraDevice->open(callback,
        [&](auto s, const auto& session){
            if(s == Status::OK){
                cameraSession = session;
                LOGI("CameraSession was acquired");
            }
            if(session == nullptr){

            }
    });

    cameraSession->constructDefaultRequestSettings(::android::hardware::camera::device::V3_2::RequestTemplate::PREVIEW,
        [&](auto status, const auto& req){
            LOGD("constructDefaultRequestSettings return : %d", status);
            if(status == Status::OK){
                // const camera_metadata* r = reinterpret_cast<const camera_metadata_t*>(req.data());
                // size_t expectedSize = req.size();
                // int ret = validate_camera_metadata_structure(r, &expectedSize);
                // if(ret == android::OK || ret == CAMERA_METADATA_VALIDATION_SHIFTED){
                //     *metadata = clone_camera_metadata(r);
                // }
                metadata = req;
            }
    });

    Stream stream = { 0, ::android::hardware::camera::device::V3_2::StreamType::OUTPUT, static_cast<uint32_t>(1280), static_cast<uint32_t>(720), ::android::hardware::graphics::common::V1_0::PixelFormat::RGBA_8888,
                        0, 0, ::android::hardware::camera::device::V3_2::StreamRotation::ROTATION_0};
    ::android::hardware::hidl_vec<Stream> streams = {stream};
    ::android::hardware::camera::device::V3_2::StreamConfiguration config = {streams, ::android::hardware::camera::device::V3_2::StreamConfigurationMode::NORMAL_MODE};
    cameraSession->configureStreams(config,
        [&](auto status, const auto& halConfig){
            if(status == Status::OK){
                halStreamConfig = halConfig;
            }
    });


    sp<GraphicBuffer> gb = new GraphicBuffer(1280, 720, android::PIXEL_FORMAT_RGB_565, 1, 
        android_convertGralloc1To0Usage(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN, android::GraphicBuffer::USAGE_SW_READ_OFTEN));
    StreamBuffer outputBuffer = {0, 0, hidl_handle(gb->getNativeBuffer()->handle), BufferStatus::OK, nullptr, nullptr};
    hidl_vec<StreamBuffer> outputBuffers = { outputBuffer };
    StreamBuffer emptyInputBuffer = {-1, 0, nullptr, BufferStatus::ERROR, nullptr, nullptr};
    CaptureRequest request = {0, 0, metadata, emptyInputBuffer, outputBuffers};
    hidl_vec<BufferCache> caches;
    uint32_t numRequestProcessed = 0;
    cameraSession->processCaptureRequest(
        {request}, caches, [&](auto status, const auto& num){
            if(status == Status::OK){
                numRequestProcessed = num;
            }
    });

    cameraSession->processCaptureRequest(
        {request}, caches, [&](auto status, const auto& num){
            if(status == Status::OK){
                numRequestProcessed = num;
            }
    });

}
