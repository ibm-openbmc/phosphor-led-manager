#include "config.h"

#include "operational-status-monitor.hpp"

#ifdef IBM_SAI
#include "ibm-sai.hpp"
#endif

#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/lg2.hpp>

#include <iostream>

namespace phosphor
{
namespace led
{
namespace Operational
{
namespace status
{
namespace monitor
{
void Monitor::removeCriticalAssociation(const std::string& objectPath,
                                        const std::string& service)
{
    try
    {
        DBusValue getAssociationValue;

        auto method =
            bus.new_method_call(service.c_str(), objectPath.c_str(),
                                "org.freedesktop.DBus.Properties", "Get");
        method.append("xyz.openbmc_project.Association.Definitions",
                      "Associations");
        auto reply = bus.call(method);

        reply.read(getAssociationValue);

        auto association = std::get<AssociationsProperty>(getAssociationValue);

        AssociationTuple critAssociation{
            "health_rollup", "critical",
            "/xyz/openbmc_project/inventory/system/chassis"};

        auto it =
            std::find(association.begin(), association.end(), critAssociation);

        if (it != association.end())
        {
            association.erase(it);
            DBusValue setAssociationValue = association;

            auto method =
                bus.new_method_call(service.c_str(), objectPath.c_str(),
                                    "org.freedesktop.DBus.Properties", "Set");

            method.append("xyz.openbmc_project.Association.Definitions",
                          "Associations", setAssociationValue);
            bus.call(method);

            std::cout << "\nRemoved critical association between " << objectPath
                      << " and /xyz/openbmc_project/inventory/system/chassis."
                      << std::endl;
        }
    }
    catch (const sdbusplus::exception::exception& e)
    {
        std::cerr << e.what();
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what();
    }
}

void Monitor::matchHandler(sdbusplus::message::message& msg)
{
    // Get the ObjectPath of the `xyz.openbmc_project.Inventory.Manager`
    // service
    std::string invObjectPath = msg.get_path();

    // Get all the properties of
    // "xyz.openbmc_project.State.Decorator.OperationalStatus" interface
    std::string interfaceName{};
    std::map<std::string, std::variant<bool>> properties;
    msg.read(interfaceName, properties);

    const auto it = properties.find("Functional");
    if (it != properties.end())
    {
        const bool* value = std::get_if<bool>(&it->second);
        if (!value)
        {
            lg2::error(
                "Faild to get the Functional property, INVENTORY_PATH = {PATH}",
                "PATH", invObjectPath);
            return;
        }

        // If the Functional value changes from false to true, remove critical
        // associations between the corresponding FRU and the chassis.
        if (!functionalValue && *value)
        {
            removeCriticalAssociation(invObjectPath,
                                      "xyz.openbmc_project.Inventory.Manager");
        }

        functionalValue = *value;

        // See if the Inventory D-Bus object has an association with LED groups
        // D-Bus object.
        auto ledGroupPath = getLedGroupPaths(invObjectPath);
        if (ledGroupPath.empty())
        {
            lg2::info(
                "The inventory D-Bus object is not associated with the LED "
                "group D-Bus object. INVENTORY_PATH = {PATH}",
                "PATH", invObjectPath);
            return;
        }

        // Update the Asserted property by the Functional property value.
        updateAssertedProperty(ledGroupPath);
    }
}

const std::vector<std::string>
    Monitor::getLedGroupPaths(const std::string& inventoryPath) const
{
    // Get endpoints from the rType
    std::string faultLedAssociation = inventoryPath + "/fault_led_group";

    // endpoint contains the vector of strings, where each string is a Inventory
    // D-Bus object that this, associated with this LED Group D-Bus object
    // pointed to by fru_fault
    PropertyValue endpoint{};

    try
    {
        endpoint = dBusHandler.getProperty(faultLedAssociation,
                                           "xyz.openbmc_project.Association",
                                           "endpoints");
    }
    catch (const sdbusplus::exception::exception& e)
    {
        lg2::error(
            "Failed to get endpoints property, ERROR = {ERROR}, PATH = {PATH}",
            "ERROR", e, "PATH", faultLedAssociation);

        return {};
    }

    auto& endpoints = std::get<std::vector<std::string>>(endpoint);

    return endpoints;
}

void Monitor::updateAssertedProperty(
    const std::vector<std::string>& ledGroupPaths)
{
    for (const auto& path : ledGroupPaths)
    {
#ifdef IBM_SAI
        if (path == phosphor::led::ibm::PARTITION_SAI ||
            path == phosphor::led::ibm::PLATFORM_SAI)
        {
            continue;
        }
#endif

        try
        {
            // Call "Group Asserted --> true" if the value of Functional is
            // false Call "Group Asserted --> false" if the value of Functional
            // is true
            PropertyValue assertedValue{!functionalValue};
            dBusHandler.setProperty(path, "xyz.openbmc_project.Led.Group",
                                    "Asserted", assertedValue);
        }
        catch (const sdbusplus::exception::exception& e)
        {
            lg2::error(
                "Failed to set Asserted property, ERROR = {ERROR}, PATH = {PATH}",
                "ERROR", e, "PATH", path);
        }
    }
}
} // namespace monitor
} // namespace status
} // namespace Operational
} // namespace led
} // namespace phosphor
