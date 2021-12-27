#pragma once

#include "group.hpp"
#include "manager.hpp"
#include "serialize.hpp"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/server/object.hpp>

#include <map>
#include <memory>
#include <string>

namespace phosphor
{
namespace led
{

using ObjectPath = std::string;

/** @class CustomDBus
 *  @brief This is a custom D-Bus object, used to add D-Bus interface and
 *         update the corresponding properties value.
 */
class CustomDBus
{
  private:
    CustomDBus()
    {}

  public:
    CustomDBus(const CustomDBus&) = delete;
    CustomDBus(CustomDBus&&) = delete;
    CustomDBus& operator=(const CustomDBus&) = delete;
    CustomDBus& operator=(CustomDBus&&) = delete;
    ~CustomDBus() = default;

    static CustomDBus& getCustomDBus()
    {
        static CustomDBus customDBus;
        return customDBus;
    }

  public:
    /** @brief Set the Asserted property
     *
     *  @param[in] path     - The object path
     *  @param[in] entity   - pldm entity
     *  @param[in] value    - To assert a group, set to True. To de-assert a
     *                        group, set to False.
     *  @param[in] hostEffecterParser    - Pointer to host effecter parser
     *  @param[in] instanceId - instance Id
     *  @param[in] isTriggerStateEffecterStates - Trigger stateEffecterStates
     *                                            command flag, true: trigger
     */
    void setLedGroup(const std::string& path, Manager& manager,
                     Serialize& serialize,
                     std::function<void(Group*, bool)> callBack = nullptr);

    void updateAsserted(const std::string& path, bool value);

  private:
    std::unordered_map<ObjectPath, std::unique_ptr<Group>> ledGroup;
};

} // namespace led
} // namespace phosphor
