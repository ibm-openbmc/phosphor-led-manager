#include "ibm-sai.hpp"

#include "utils.hpp"

#include <phosphor-logging/lg2.hpp>

#include <iostream>

namespace phosphor
{
namespace led
{
namespace ibm
{
// Set OperationalStatus functional according to the asserted state of the group
void setOperationalStatus(const std::string& path, bool value)
{
    if (path != PARTITION_SAI && path != PLATFORM_SAI)
    {
        return;
    }

    // Get endpoints from the rType
    std::string fru = path + "/fault_inventory_object";

    // endpoint contains the vector of strings, where each string is a Inventory
    // D-Bus object that this, associated with this LED Group D-Bus object
    // pointed to by fru_fault
    utils::PropertyValue endpoint{};

    try
    {
        endpoint = utils::DBusHandler().getProperty(
            fru, "xyz.openbmc_project.Association", "endpoints");
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        lg2::error(
            "Failed to get endpoints property, ERROR = {ERROR}, PATH = {PATH}",
            "ERROR", e.what(), "PATH", fru);
        return;
    }

    auto& endpoints = std::get<std::vector<std::string>>(endpoint);
    if (endpoints.empty())
    {
        return;
    }

    for (const auto& fruInstancePath : endpoints)
    {
        std::cout << "SAI: FRU path: " << fruInstancePath << std::endl;
        // Set OperationalStatus by fru instance path
        try
        {
            utils::PropertyValue functionalValue{value};
            utils::DBusHandler().setProperty(
                fruInstancePath,
                "xyz.openbmc_project.State.Decorator.OperationalStatus",
                "Functional", functionalValue);
        }
        catch (const sdbusplus::exception::SdBusError& e)
        {
            lg2::error(
                "Failed to set Functional property, ERROR = {ERROR}, PATH = {PATH}",
                "ERROR", e.what(), "PATH", fruInstancePath);
        }
    }
}

} // namespace ibm
} // namespace led
} // namespace phosphor
