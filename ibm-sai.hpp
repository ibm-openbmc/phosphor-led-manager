#pragma once
#include <sdbusplus/server.hpp>

#include <map>
#include <vector>
namespace phosphor
{
namespace led
{
namespace ibm
{
constexpr auto PARTITION_SAI =
    "/xyz/openbmc_project/led/groups/partition_system_attention_indicator";
constexpr auto PLATFORM_SAI =
    "/xyz/openbmc_project/led/groups/platform_system_attention_indicator";

/** @brief Set OperationalStatus according to the status of asserted
 *         property
 *
 *  @param[in]  path          -  D-Bus path of group
 *  @param[in]  value         -  Could be true or false
 *
 *  @return: None
 */
void setOperationalStatus(const std::string& path, bool value);

} // namespace ibm
} // namespace led
} // namespace phosphor
