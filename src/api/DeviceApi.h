//DeviceApi.h
#pragma once

namespace overseer::device::api {

class DeviceApi {
public:
    virtual void begin() = 0;
    virtual void broadcast() = 0;
    virtual ~DeviceApi() {}
};

} // namespace api
