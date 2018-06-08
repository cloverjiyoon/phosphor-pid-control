#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>

#include "sensors/sensor.hpp"


/*
 * The SensorManager holds all sensors across all zones.
 */
class SensorManager
{
    public:
        SensorManager(sdbusplus::bus::bus&& pass, sdbusplus::bus::bus&& host)
            : _passiveListeningBus(std::move(pass)),
              _hostSensorBus(std::move(host))
        {
            // manager gets its interface from the bus. :D
            sdbusplus::server::manager::manager(_hostSensorBus, SensorRoot);
        }

        SensorManager()
            : SensorManager(std::move(sdbusplus::bus::new_default()),
                            std::move(sdbusplus::bus::new_default()))
        {
        }

        ~SensorManager() = default;
        SensorManager(const SensorManager&) = delete;
        SensorManager& operator=(const SensorManager&) = delete;
        SensorManager(SensorManager&&) = default;
        SensorManager& operator=(SensorManager&&) = default;

        /*
         * Add a Sensor to the Manager.
         */
        void addSensor(
            std::string type,
            std::string name,
            std::unique_ptr<Sensor> sensor);

        // TODO(venture): Should implement read/write by name.
        const std::unique_ptr<Sensor>& getSensor(const std::string& name) const
        {
            return _sensorMap.at(name);
        }

        sdbusplus::bus::bus& getPassiveBus(void)
        {
            return _passiveListeningBus;
        }

        sdbusplus::bus::bus& getHostBus(void)
        {
            return _hostSensorBus;
        }

    private:
        std::map<std::string, std::unique_ptr<Sensor>> _sensorMap;
        std::map<std::string, std::vector<std::string>> _sensorTypeList;

        sdbusplus::bus::bus _passiveListeningBus;
        sdbusplus::bus::bus _hostSensorBus;

        static constexpr auto SensorRoot = "/xyz/openbmc_project/extsensors";
};
