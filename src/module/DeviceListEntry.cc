#include "DeviceListEntry.h"
#include "Device.h"

using namespace Napi;

DeviceListEntry::DeviceListEntry(const CallbackInfo &info) : ObjectWrap<DeviceListEntry>(info) {
    listRef = Persistent(info[0]);
    LIBMTP_raw_device_t* rawdevices = info[0].As<External<LIBMTP_raw_device_t>>().Data();
    int i = info[1].As<Number>();
    rawDevice = &rawdevices[i];
    deviceEntry = &rawdevices[i].device_entry;
}

DeviceListEntry::~DeviceListEntry() {
    fprintf(stderr, "DeviceListEntry::~DeviceListEntry called\n");
}

FunctionReference DeviceListEntry::constructor;

Object DeviceListEntry::NewInstance(Napi::Value deviceListExternal, Number i) {
    return constructor.New({ deviceListExternal, i });
}

void DeviceListEntry::Init(Napi::Env env) {
    Function func = DefineClass(env, "DeviceListEntry", {
        InstanceMethod<&DeviceListEntry::Attach>("attach"),
        InstanceAccessor<&DeviceListEntry::GetVendor>("vendor"),
        InstanceAccessor<&DeviceListEntry::GetProduct>("product")
    });

    constructor = Persistent(func);
    constructor.SuppressDestruct();
}

Value DeviceListEntry::Attach(const CallbackInfo &info) {

    LIBMTP_mtpdevice_t* device = LIBMTP_Open_Raw_Device_Uncached(rawDevice);
    if(device == NULL) {
        return info.Env().Null();
    }
    Napi::Value extDevice = External<LIBMTP_mtpdevice_t>::New(info.Env(), device);
    return Device::NewInstance(extDevice);
}

Value DeviceListEntry::GetVendor(const CallbackInfo &info) {
    return String::New(info.Env(), this->deviceEntry->vendor);
}

Value DeviceListEntry::GetProduct(const CallbackInfo &info) {
    return String::New(info.Env(), this->deviceEntry->product);
}
