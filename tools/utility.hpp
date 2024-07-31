#pragma once
#include <sdbusplus/server.hpp>

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace utility
{

/**
 * @brief An API to make GetSubTree mapper call.
 *
 * @return MapperResponse - Response from the API.
 */
// map<inventory_path, map<service_name, vector<interface>>>
using MapperResponse =
    std::map<std::string, std::map<std::string, std::vector<std::string>>>;

MapperResponse
    getObjectSubtreeForInterfaces(const std::string& root, const int32_t depth,
                                  const std::vector<std::string>& interfaces)
{
    auto bus = sdbusplus::bus::new_default();
    auto mapperCall = bus.new_method_call("xyz.openbmc_project.ObjectMapper",
                                          "/xyz/openbmc_project/object_mapper",
                                          "xyz.openbmc_project.ObjectMapper",
                                          "GetSubTree");
    mapperCall.append(root);
    mapperCall.append(depth);
    mapperCall.append(interfaces);

    MapperResponse result = {};

    try
    {
        auto response = bus.call(mapperCall);

        response.read(result);
    }
    catch (const sdbusplus::exception_t& e)
    {
        std::cout << "GetSubtree mapper call failed with exception: "
                  << e.what() << std::endl;
    }
    return result;
}

/**
 * @brief An API to get associated object paths.
 *
 * @param[in] associationPath - The path to look for the association endpoints.
 * @param[in] root - The root of the tree to look into.
 * @param[in] depth - The maximum depth of the tree past the root to search.
 * @param[in] interfaces - Optional list of interfaces to constrain the search.
 * @return ListOfObjectPaths - List of object paths associted.
 */
using ListOfObjectPaths = std::vector<std::string>;
ListOfObjectPaths GetAssociatedSubTreePaths(
    const sdbusplus::message::object_path& associationPath,
    const sdbusplus::message::object_path& root, const int32_t depth,
    const std::vector<std::string>& interfaces)
{
    auto bus = sdbusplus::bus::new_default();
    auto mapperCall = bus.new_method_call("xyz.openbmc_project.ObjectMapper",
                                          "/xyz/openbmc_project/object_mapper",
                                          "xyz.openbmc_project.ObjectMapper",
                                          "GetAssociatedSubTreePaths");
    mapperCall.append(associationPath);
    mapperCall.append(root);
    mapperCall.append(depth);
    mapperCall.append(interfaces);

    ListOfObjectPaths result = {};

    try
    {
        auto response = bus.call(mapperCall);

        response.read(result);
    }
    catch (const sdbusplus::exception_t& e)
    {
        std::cerr
            << "GetAssociatedSubTreePaths mapper call failed with exception: "
            << e.what() << std::endl;
    }
    return result;
}

/**
 * @brief Templated API to set D-Bus property.
 *
 * @param[in] serviceName - D-Bus service name hosting the object path.
 * @param[in] objectPath - D-Bus object path hosting the interface.
 * @param[in] interface - D-Bus interface hosting the property.
 * @param[in] property - D-Bus property to be set.
 * @param[in] value - Value to be set.
 */
template <typename T>
void setProperty(const std::string& serviceName, const std::string& objectPath,
                 const std::string& interface, const std::string& property,
                 const std::variant<T>& value)
{
    try
    {
        auto bus = sdbusplus::bus::new_default();
        auto method =
            bus.new_method_call(serviceName.c_str(), objectPath.c_str(),
                                "org.freedesktop.DBus.Properties", "Set");

        method.append(interface);
        method.append(property);
        method.append(value);
        bus.call(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::cerr << "Set property: " << property
                  << " failed for path: " << objectPath
                  << " with error: " << e.what() << std::endl;
    }
}

/**
 * @brief Templated API to get D-Bus property.
 *
 * @param[in] serviceName - D-Bus service name hosting the object path.
 * @param[in] objectPath - D-Bus object path hosting the interface.
 * @param[in] interface - D-Bus interface hosting the property.
 * @param[in] property - D-Bus property whose value is to be fetched.
 * @return Value of the property.
 */
template <typename T>
T getProperty(const std::string& serviceName, const std::string& objectPath,
              const std::string& interface, const std::string& property)
{
    auto bus = sdbusplus::bus::new_default();
    auto mapperCall =
        bus.new_method_call(serviceName.c_str(), objectPath.c_str(),
                            "org.freedesktop.DBus.Properties", "Get");
    mapperCall.append(interface);
    mapperCall.append(property);

    T result = {};

    try
    {
        auto response = bus.call(mapperCall);

        response.read(result);
    }
    catch (const sdbusplus::exception_t& e)
    {
        std::cerr << "Get property: " << property
                  << " failed for path: " << objectPath
                  << " with error: " << e.what() << std::endl;
    }

    return result;
}

/**
 * @brief An API to check chassis power state.
 *
 * @return bool - True when chassis power state is on, false otherwise.
 */
bool isChassisOn()
{
    auto retVal = getProperty<std::variant<std::string>>(
        "xyz.openbmc_project.State.Chassis0",
        "/xyz/openbmc_project/state/chassis0",
        "xyz.openbmc_project.State.Chassis", "CurrentPowerState");

    if (auto powerSate = std::get_if<std::string>(&retVal))
    {
        if (*powerSate == "xyz.openbmc_project.State.Chassis.PowerState.On")
        {
            return true;
        }
    }

    // In any error condition assuming chassis is off.
    return false;
}
} // namespace utility
