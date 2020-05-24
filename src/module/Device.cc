#include "Device.h"

using namespace Napi;

#define stringGetterFromFunc(name, func) \
    Value Device::name(const CallbackInfo &info) { \
        char * val = func(device); \
        if(val == NULL) { \
            return info.Env().Null(); \
        } \
        Napi::Value res = String::New(info.Env(), val); \
        free(val); \
        return res; \
    }

Device::Device(const CallbackInfo &info) : ObjectWrap<Device>(info) {
    device = info[0].As<External<LIBMTP_mtpdevice_t>>().Data();
}

Device::~Device() {
    LIBMTP_Release_Device(device);
    fprintf(stderr, "Device::~Device called\n");
}

FunctionReference Device::constructor;

Object Device::NewInstance(Napi::Value device) {
    return constructor.New({ device });
}

void Device::Init(Napi::Env env) {
    Function func = DefineClass(env, "Device", {
        InstanceMethod<&Device::Attach>("attach"),
        InstanceAccessor<&Device::GetBatteryLevel>("batteryLevel", napi_enumerable),
        InstanceAccessor<&Device::GetManufacturer>("manufacturer", napi_enumerable),
        InstanceAccessor<&Device::GetModel>("model", napi_enumerable),
        InstanceAccessor<&Device::GetName>("name", napi_enumerable),
        InstanceAccessor<&Device::GetVersion>("version", napi_enumerable),
        InstanceAccessor<&Device::GetSerial>("serial", napi_enumerable),
    });

    constructor = Persistent(func);
    constructor.SuppressDestruct();
}

stringGetterFromFunc(GetManufacturer, LIBMTP_Get_Manufacturername)
stringGetterFromFunc(GetModel, LIBMTP_Get_Modelname)
stringGetterFromFunc(GetName, LIBMTP_Get_Friendlyname)
stringGetterFromFunc(GetVersion, LIBMTP_Get_Deviceversion)
stringGetterFromFunc(GetSerial, LIBMTP_Get_Serialnumber)

Value Device::GetBatteryLevel(const CallbackInfo &info) {
    uint8_t maxbattlevel;
    uint8_t currbattlevel;

    if(LIBMTP_Get_Batterylevel(this->device, &maxbattlevel, &currbattlevel) == 0) {
        return Number::New(info.Env(), ((float) currbattlevel/ (float) maxbattlevel));
    }

    // Silently ignore. Some devices does not support getting the
    // battery level.
    LIBMTP_Clear_Errorstack(device);
    return info.Env().Null();
}
