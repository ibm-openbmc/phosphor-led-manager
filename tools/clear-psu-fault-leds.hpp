#pragma once

#include "utility.hpp"

#include <unistd.h>

/**
 * @brief API to clear PSU fault LEDs.
 *
 * The function, sets operational status of powersupply FRU to true as well as
 * corresponding asserted property of fault led associated with that FRU to
 * false.
 *
 * Note: The retry mechanishm is to wait for LED service to come up without
 * which the call to fetch endpoints would fail.
 * Explicit dependency for LED is not added in service as it will delay the
 * execution further and clearing of PSU LED should happen asap.
 */
void clearPsuFaultLeds()
{
    // sufficiently large number within which the LED service should come up.
    static constexpr auto MAX_RETRY = 120;

    if (utility::isChassisOn())
    {
        std::cout
            << "Abort clearing PSU fault LED. Chassis is in power on state."
            << std::endl;

        return;
    }

    std::vector<std::string> interfaces{
        "xyz.openbmc_project.Inventory.Item.PowerSupply"};

    utility::MapperResponse subTree = utility::getObjectSubtreeForInterfaces(
        "/xyz/openbmc_project/inventory", 0, interfaces);

    if (subTree.size() == 0)
    {
        std::cerr << "No sub tree found for power supply interfaces. Exiting."
                  << std::endl;

        return;
    }

    for (const auto& [objectPath, serviceInterfaceMap] : subTree)
    {
        // Find the service which hosts the current objectPath.
        if (!serviceInterfaceMap.contains(
                "xyz.openbmc_project.Inventory.Manager"))
        {
            // If this object path is not hosted by inventory manager service,
            // do not take any action.
            continue;
        }

        auto retryCounter = MAX_RETRY;

        while (retryCounter != 0)
        {
            utility::ListOfObjectPaths objPathList =
                utility::GetAssociatedSubTreePaths(
                    std::string(objectPath + "/fault_identifying"),
                    std::string("/xyz/openbmc_project/led/groups"), 0,
                    std::vector<std::string>{});

            // If path list is empty, implies LED service is not yet up. We
            // need to wait for the service.
            if (objPathList.size() == 0)
            {
                retryCounter--;
                sleep(1);
                continue;
            }
            break;
        }

        // timer exhausted. Flag the error.
        if (retryCounter == 0)
        {
            std::cerr << "Could not find associated object path for: "
                      << objectPath << " Exiting." << std::endl;
            return;
        }

        utility::setProperty<bool>(
            "xyz.openbmc_project.Inventory.Manager", objectPath,
            "xyz.openbmc_project.State.Decorator.OperationalStatus",
            "Functional", true);

        auto retVal =
            utility::getProperty<std::variant<std::vector<std::string>>>(
                "xyz.openbmc_project.ObjectMapper",
                objectPath + "/fault_identifying",
                "xyz.openbmc_project.Association", "endpoints");

        if (auto endpoints = std::get_if<std::vector<std::string>>(&retVal))
        {
            for (const auto& path : *endpoints)
            {
                // Asserted is inverse of functional status.
                utility::setProperty<bool>(
                    "xyz.openbmc_project.LED.GroupManager", path,
                    "xyz.openbmc_project.Led.Group", "Asserted", false);
            }

            std::cout << "Setting endpoint for object path: " << objectPath
                      << " completed in " << (MAX_RETRY - retryCounter)
                      << " seconds." << std::endl;
        }
        else
        {
            std::cout << "No endpoints for path: " << objectPath << std::endl;
        }
    }
}
