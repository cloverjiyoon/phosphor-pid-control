/**
 * Copyright 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <chrono>
#include <cmath>
#include <mutex>

#include "dbuspassive.hpp"

DbusPassive::DbusPassive(
    sdbusplus::bus::bus& bus,
    const std::string& type,
    const std::string& id)
    : ReadInterface(),
      _bus(bus),
      _signal(bus, GetMatch(type, id).c_str(), DbusHandleSignal, this),
      _id(id)
{
    /* Need to get the scale and initial value */
    auto tempBus = sdbusplus::bus::new_default();
    /* service == busname */
    std::string path = GetSensorPath(type, id);
    std::string service = GetService(tempBus, sensorintf, path);

    struct SensorProperties settings;
    GetProperties(tempBus, service, path, &settings);

    _scale = settings.scale;
    _value = settings.value * pow(10, _scale);
    _updated = std::chrono::high_resolution_clock::now();
}

ReadReturn DbusPassive::read(void)
{
    std::lock_guard<std::mutex> guard(_lock);

    struct ReadReturn r = {
        _value,
        _updated
    };

    return r;
}

void DbusPassive::setValue(double value)
{
    std::lock_guard<std::mutex> guard(_lock);

    _value = value;
    _updated = std::chrono::high_resolution_clock::now();
}

int64_t DbusPassive::getScale(void)
{
    return _scale;
}

std::string DbusPassive::getId(void)
{
    return _id;
}

int DbusHandleSignal(sd_bus_message* msg, void* usrData, sd_bus_error* err)
{
    namespace sdm = sdbusplus::message;
    auto sdbpMsg = sdm::message(msg);
    DbusPassive* obj = static_cast<DbusPassive*>(usrData);

    std::string msgSensor;
    std::map<std::string, sdm::variant<int64_t>> msgData;
    sdbpMsg.read(msgSensor, msgData);

    if (msgSensor == "xyz.openbmc_project.Sensor.Value")
    {
        auto valPropMap = msgData.find("Value");
        if (valPropMap != msgData.end())
        {
            int64_t rawValue = sdm::variant_ns::get<int64_t>
                               (valPropMap->second);

            double value = rawValue * pow(10, obj->getScale());

#if 0
            std::cerr << "received update: " << value
                      << " for: " << obj->getId()
                      << std::endl;
#endif

            obj->setValue(value);
        }
    }

    return 0;
}