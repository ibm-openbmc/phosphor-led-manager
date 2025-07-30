#pragma once
#include "utility.hpp"

/**
 * @brief API to dump all the led object paths.
 */
void dumpLEDObjectPaths()
{
    std::vector<std::string> interfaces;

    const utility::MapperResponse subTree =
        utility::getObjectSubtreeForInterfaces(
            "/xyz/openbmc_project/led/groups", 0, interfaces);

    if (subTree.empty())
    {
        std::cout << "No sub tree found for led groups. Exiting." << std::endl;
        return;
    }

    std::cout << "xyz.openbmc_project.LED.GroupManager" << "\n";
    for (const auto& [objectPath, serviceInterfaceMap] : subTree)
    {
        if (serviceInterfaceMap.find("xyz.openbmc_project.LED.GroupManager") !=
            serviceInterfaceMap.end())
        {
            std::cout << "    " << objectPath << "\n";
        }
    }
}
