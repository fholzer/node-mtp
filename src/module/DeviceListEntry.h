#include <napi.h>
#include "libmtp.h"

class DeviceListEntry : public Napi::ObjectWrap<DeviceListEntry> {
    public:
        static void Init(Napi::Env env);

        DeviceListEntry(const Napi::CallbackInfo &info);

        ~DeviceListEntry();
        static Napi::Object NewInstance(Napi::Value, Napi::Number);

    private:
        Napi::Value Attach(const Napi::CallbackInfo &info);
        static Napi::FunctionReference constructor;
        LIBMTP_device_entry_t* deviceEntry;
        Napi::Reference<Napi::Value> listRef;
        LIBMTP_raw_device_t* rawDevice;
        Napi::Value GetVendor(const Napi::CallbackInfo &info);
        Napi::Value GetProduct(const Napi::CallbackInfo &info);
};
