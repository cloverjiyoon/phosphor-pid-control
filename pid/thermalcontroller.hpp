#pragma once

#include <memory>
#include <string>
#include <vector>

#include "controller.hpp"
#include "ec/pid.hpp"


/*
 * A ThermalController is a PID controller that reads a number of sensors and
 * provides the set-points for the fans.
 */
class ThermalController : public PIDController
{
    public:
        static std::unique_ptr<PIDController> CreateThermalPid(
            std::shared_ptr<PIDZone> owner,
            const std::string& id,
            std::vector<std::string>& inputs,
            float setpoint,
            ec::pidinfo initial);

        ThermalController(const std::string& id,
                          std::vector<std::string>& inputs,
                          std::shared_ptr<PIDZone> owner)
            : PIDController(id, owner),
              _inputs(inputs)
        { }

        float input_proc(void) override;
        float setpt_proc(void) override;
        void output_proc(float value) override;

    private:
        std::vector<std::string> _inputs;
};
