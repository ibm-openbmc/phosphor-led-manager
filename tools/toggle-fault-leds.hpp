#pragma once
#include "tools-utility.hpp"

/**
 * @brief API to toggle all fault LEDs according to the given functional status.
 *
 * The API sets "Functional" property hosted by operational status interface for
 * all the FRUs under root path "/xyz/openbmc_project/inventory". It also
 * fetches associated LEDs object path for FRUs, hosted under LED manager and
 * sets asserted vlaue for accordingly.
 *
 * The API skips some specific FRUs, although they host operational status based
 * on following criteria.
 * - Can be a guarded FRU.
 * - Operational status of the FRU is handled by some other module. We don't
 * want conflict there.
 *
 * @param[in] isFunctional - States if the FRUs are functional or not.
 */
void toggleFaultLeds(const bool isFunctional)
{
    std::cout << "Trigger toggling of fault LEDs with value: " << isFunctional
              << std::endl;

    std::vector<std::string> skipOperationalStatusForFRUs{
        "core", "powersupply", "unit", "connector", "chassis1"};

    std::vector<std::string> frusWithoutLED{
        "pcie_card", "usb",    "drive",       "ethernet",  "fan0_",
        "fan1_",     "fan2_",  "fan3_",       "fan4_",     "fan5_",
        "rdx",       "cables", "displayport", "pcieslot12"};

    if (utility::isChassisOn())
    {
        std::cout
            << "Abort toggling of fault LED script as chassis is in power on state."
            << std::endl;

        return;
    }

    std::vector<std::string> interfaces{
        "xyz.openbmc_project.State.Decorator.OperationalStatus"};

    utility::MapperResponse subTree = utility::getObjectSubtreeForInterfaces(
        "/xyz/openbmc_project/inventory", 0, interfaces);

    if (subTree.size() == 0)
    {
        std::cout
            << "No sub tree found for OperationalStatus interfaces. Exiting."
            << std::endl;

        return;
    }

    for (const auto& [objectPath, serviceInterfceMap] : subTree)
    {
        if (std::any_of(skipOperationalStatusForFRUs.cbegin(),
                        skipOperationalStatusForFRUs.cend(),
                        [&objectPath](const std::string& str) {
            return objectPath.find(str) != std::string::npos;
            }))
        {
            //"Skip setting operational status for these paths."
            continue;
        }

        utility::setProperty<bool>(
            "xyz.openbmc_project.Inventory.Manager", objectPath,
            "xyz.openbmc_project.State.Decorator.OperationalStatus",
            "Functional", isFunctional);

        if (std::any_of(frusWithoutLED.cbegin(), frusWithoutLED.cend(),
                        [&objectPath](const std::string& str) {
            return objectPath.find(str) != std::string::npos;
            }))
        {
            //"Skipping path with no LEDs."
            continue;
        }

        auto retVal =
            utility::getProperty<std::variant<std::vector<std::string>>>(
                "xyz.openbmc_project.ObjectMapper",
                objectPath + "/fault_identifying",
                "xyz.openbmc_project.Association", "endpoints");

        if (auto endpoints = std::get_if<std::vector<std::string>>(&retVal))
        {
            bool isAsserted = false;

            if (!isFunctional)
            {
                isAsserted = true;
            }
            
            for (const auto& path : *endpoints)
            {
                utility::setProperty<bool>(
                    "xyz.openbmc_project.LED.GroupManager", path,
                    "xyz.openbmc_project.Led.Group", "Asserted", isAsserted);
            }
        }
        else
        {
            std::cout << "No Endpoints for path: " << objectPath << std::endl;
        }
    }
}