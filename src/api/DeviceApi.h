//DeviceApi.h
#pragma once

namespace api {

class DeviceApi {
public:
    virtual void begin() = 0;
    virtual void broadcast() = 0;
    virtual ~DeviceApi() {}
};

} // namespace api
