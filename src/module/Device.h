#include <napi.h>
#include "libmtp.h"

class Device : public Napi::ObjectWrap<Device> {
    public:
        static void Init(Napi::Env env);

        Device(const Napi::CallbackInfo &info);

        ~Device();
        static Napi::Object NewInstance(Napi::Value);

    private:
        Napi::Value Attach(const Napi::CallbackInfo &info);
        static Napi::FunctionReference constructor;
        LIBMTP_mtpdevice_t* device;
        Napi::Reference<Napi::Value> listRef;
        Napi::Value GetBatteryLevel(const Napi::CallbackInfo &info);
        Napi::Value GetManufacturer(const Napi::CallbackInfo &info);
        Napi::Value GetModel(const Napi::CallbackInfo &info);
        Napi::Value GetName(const Napi::CallbackInfo &info);
        Napi::Value GetVersion(const Napi::CallbackInfo &info);
        Napi::Value GetSerial(const Napi::CallbackInfo &info);
};
