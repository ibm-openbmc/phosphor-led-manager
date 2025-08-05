#pragma once
#include "utility.hpp"

/**
 * @brief API to toggle fault LEDs according to the given functional status.
 *
 * The API sets "Functional" property hosted by operational status interface for
 * all the FRUs under root path "/xyz/openbmc_project/inventory". It also
 * fetches associated LEDs object path for FRUs, hosted under LED manager and
 * sets asserted value accordingly.
 *
 * The API skips some specific FRUs, although they host operational status based
 * on following criteria.
 * - Can be a guarded FRU.
 * - Operational status of the FRU is handled by some other module.
 *
 * @param[in] isFunctional - States if the FRUs are functional or not.
 * @param[in] overrideChassisOnCheck - Flag to override chassis on check.
 */
void toggleFaultLeds(const bool isFunctional,
                     const bool overrideChassisOnCheck = false)
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
        if (!overrideChassisOnCheck)
        {
            std::cout
                << "Abort toggling fault LEDs. Chassis is in power on state."
                << std::endl;

            return;
        }

        utility::createPel(
            __FILE__, __FUNCTION__, "Toggle Fault LEDs while chassis is on",
            "xyz.openbmc_project.Common.Error.NotAllowed",
            "xyz.openbmc_project.Logging.Entry.Level.Informational");
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

    for (const auto& [objectPath, serviceInterfaceMap] : subTree)
    {
        // only handle paths which are hosted by PIM
        if (!serviceInterfaceMap.contains(
                "xyz.openbmc_project.Inventory.Manager"))
        {
            // If this object path is not hosted by inventory manager service,
            // do not take any action.
            continue;
        }

        if (std::any_of(skipOperationalStatusForFRUs.cbegin(),
                        skipOperationalStatusForFRUs.cend(),
                        [&objectPath](const std::string& aFru) {
            return objectPath.find(aFru) != std::string::npos;
        }))
        {
            // Skip setting operational status for these paths.
            continue;
        }

        utility::notifyPIM(
            {{objectPath,
              {{"xyz.openbmc_project.State.Decorator.OperationalStatus",
                {{"Functional", isFunctional}}}}}});

        if (std::any_of(frusWithoutLED.cbegin(), frusWithoutLED.cend(),
                        [&objectPath](const std::string& aFru) {
            // should skip for pcie_cards base path and connectors, not for cxp
            // ports as they have LEDs
            if (objectPath.find("cxp_top") != std::string::npos ||
                objectPath.find("cxp_bot") != std::string::npos)
            {
                return false;
            }
            return objectPath.find(aFru) != std::string::npos;
        }))
        {
            // Skipping path with no LEDs.
            continue;
        }

        auto retVal = utility::getProperty("xyz.openbmc_project.ObjectMapper",
                                           objectPath + "/fault_identifying",
                                           "xyz.openbmc_project.Association",
                                           "endpoints");

        if (auto endpoints = std::get_if<std::vector<std::string>>(&retVal))
        {
            for (const auto& path : *endpoints)
            {
                // Asserted is inverse of functional status.
                utility::setProperty<bool>(
                    "xyz.openbmc_project.LED.GroupManager", path,
                    "xyz.openbmc_project.Led.Group", "Asserted", !isFunctional);
            }
        }
        else
        {
            std::cout << "No endpoints for path: " << objectPath << std::endl;
        }
    }
}
