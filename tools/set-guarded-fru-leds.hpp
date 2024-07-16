#pragma once
#include "utility.hpp"

/**
 * @brief API to set LEDs for guarded FRUs.
 *
 * As per current design. All the fault LEDs associated with FRUs gets cleared
 * on a BMC reboot at chassis off state. As a part of this process, to avoid
 * losing LEDs which were lit up due to guard records, the function needs to be
 * executed.
 * It checks for existing guard records and set LEDs asserted state for them as
 * true if required.
 */
void setLEDForGuardedFru()
{
    std::cout << "Trigger set LED for guarded FRUs" << std::endl;

    if (utility::isChassisOn())
    {
        std::cout
            << "Abort set led for guarded FRUs as chassis is in power on state."
            << std::endl;

        return;
    }

    std::vector<std::string> interfaces{
        "xyz.openbmc_project.Association.Definitions"};

    utility::MapperResponse subTree = utility::getObjectSubtreeForInterfaces(
        "/xyz/openbmc_project/hardware_isolation/entry", 0, interfaces);

    if (subTree.size() == 0)
    {
        std::cout << "No sub tree found for guarded FRUs. Exiting."
                  << std::endl;

        return;
    }

    for (const auto& [objectPath, serviceInterfceMap] : subTree)
    {
        auto retVal = utility::getProperty<std::variant<
            std::vector<std::tuple<std::string, std::string, std::string>>>>(
            "org.open_power.HardwareIsolation", objectPath,
            "xyz.openbmc_project.Association.Definitions", "Associations");

        if (auto listOfPaths = std::get_if<
                std::vector<std::tuple<std::string, std::string, std::string>>>(
                &retVal))
        {
            for (const auto& aPath : (*listOfPaths))
            {
                const std::string& guardedPath = std::get<2>(aPath);

                if (guardedPath.find("dimm") != std::string::npos)
                {
                    utility::setProperty<bool>(
                        "xyz.openbmc_project.Inventory.Manager", guardedPath,
                        "xyz.openbmc_project.State.Decorator.OperationalStatus",
                        "Functional", false);
                    continue;
                }

                if (guardedPath.find("cpu") != std::string::npos)
                {
                    // Not checking for core as that is hosted under PLDM not
                    // PIM. Which is not tied to LEDs.
                    if (guardedPath.find("unit") != std::string::npos)
                    {
                        utility::setProperty<bool>(
                            "xyz.openbmc_project.Inventory.Manager",
                            guardedPath,
                            "xyz.openbmc_project.State.Decorator.OperationalStatus",
                            "Functional", false);
                    }
                }
            }
        }
    }
}
