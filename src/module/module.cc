#include <napi.h>
#include "libmtp.h"
#include "DeviceListEntry.h"
#include "Device.h"

using namespace Napi;

void FinalizerDeviceList(napi_env env, LIBMTP_raw_device_t* finalize_data) {
    fprintf(stderr, "#### DEVICE LIST FINALIZER ####\n");
    free(finalize_data);
}

class DeviceListWorker : public AsyncWorker {
    public:
    DeviceListWorker(Function& callback)
    : AsyncWorker(callback) {}

    ~DeviceListWorker() {}

    void Execute() override {
        LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawdevices, &numrawdevices);
        switch(err) {
        case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
            return;
        case LIBMTP_ERROR_CONNECTING:
            errCode = "LIBMTP_ERROR_CONNECTING";
            SetError("There has been an error connecting.");
            return;
        case LIBMTP_ERROR_MEMORY_ALLOCATION:
            errCode = "LIBMTP_ERROR_MEMORY_ALLOCATION";
            SetError("Encountered a Memory Allocation Error.");
            return;
        case LIBMTP_ERROR_NONE:
            break;
        case LIBMTP_ERROR_GENERAL:
        default:
            errCode = "LIBMTP_ERROR_GENERAL";
            SetError("Unknown connection error.");
            return;
        }
    }

    void OnOK() override {
        Napi::Env env = Env();
        Value list = External<LIBMTP_raw_device_t>::New(env, rawdevices, &FinalizerDeviceList);
        Array arr = Array::New(env);
        int i;
        for (i = 0; i < numrawdevices; i++) {
            arr.Set(i, DeviceListEntry::NewInstance(list, Number::New(env, i)));
        }
        HandleScope scope(Env());
        Callback().Call({Env().Null(), arr});
    }

private:
    LIBMTP_raw_device_t* rawdevices = NULL;
    int numrawdevices = 0;
    const char* errCode = NULL;
};

Value getDevices(const CallbackInfo& info) {
    Function cb = info[0].As<Function>();
    DeviceListWorker* wk = new DeviceListWorker(cb);
    wk->Queue();
    return info.Env().Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("getDevices", Napi::Function::New<getDevices>(env));
    DeviceListEntry::Init(env);
    Device::Init(env);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
