#pragma once
#include "utility.hpp"

/**
 * @brief API to set asserted property to false.
 *
 * @param[in] ObjectPath - LEDs D-Bus path.
 */
void setAssertedToFalse(const std::string ObjectPath)
{
    utility::setProperty<bool>("xyz.openbmc_project.LED.GroupManager",
                               ObjectPath, "xyz.openbmc_project.Led.Group",
                               "Asserted", false);
}

/**
 * @brief Sets default state for some of the LEDs.
 *
 * The asserted property for SAI and enclosure fault LED needs to be reset at
 * every reboot so that in case of any new PELs that require to trigger SAI, the
 * property can be set and a signal can be emitted for physical LEDs to behave
 * accordingly. Enclosure identify - need not be persisted.
 * Explicitly Asserted being set as there is no operational status associated
 * these LEDs.
 */
void setLedsDefaultState()
{
    std::cout << "Trigger set default state for LEDs." << std::endl;

    if (utility::isChassisOn())
    {
        std::cout
            << "Abort setting LEDs to default as chassis is in power on state."
            << std::endl;

        return;
    }

    // set default state for power button.
    utility::setProperty<std::string>(
        "xyz.openbmc_project.LED.Controller.pca955x_front_sys_pwron0",
        "/xyz/openbmc_project/led/physical/pca955x_front_sys_pwron0",
        "xyz.openbmc_project.Led.Physical", "State",
        "xyz.openbmc_project.Led.Physical.Action.Blink");

    // set default state for enclosure fault.
    setAssertedToFalse("/xyz/openbmc_project/led/groups/enclosure_fault");

    // set default state for Platform SAI.
    setAssertedToFalse(
        "/xyz/openbmc_project/led/groups/platform_system_attention_indicator");

    // set default state for Partition SAI.
    setAssertedToFalse(
        "/xyz/openbmc_project/led/groups/partition_system_attention_indicator");

    // set default state for enclosure identify.
    setAssertedToFalse("/xyz/openbmc_project/led/groups/enclosure_identify");
}
