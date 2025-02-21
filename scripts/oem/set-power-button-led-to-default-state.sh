#!/bin/sh

# This shell script initialises Power Button LED to its default state.

# Skip running the script if chassis is powered ON.
current_chassis_status=$(busctl get-property xyz.openbmc_project.State.Chassis /xyz/openbmc_project/state/chassis0 xyz.openbmc_project.State.Chassis CurrentPowerState | cut -d" " -f2)

if [ "${current_chassis_status}" = "\"xyz.openbmc_project.State.Chassis.PowerState.On\"" ]; then
    echo "Current chassis power state is , $current_chassis_status . Exit set-power-button-led-to-default-state.sh script successfully without initialising the LEDs"
    exit 0
fi

# Explicitly set Blink as the default state
busctl set-property xyz.openbmc_project.LED.Controller.pca955x_front_sys_pwron0 /xyz/openbmc_project/led/physical/pca955x_front_sys_pwron0 xyz.openbmc_project.Led.Physical State s "xyz.openbmc_project.Led.Physical.Action.Blink"
