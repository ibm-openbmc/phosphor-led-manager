#include "custom_dbus.hpp"

namespace phosphor
{
namespace led
{

void CustomDBus::setLedGroup(const std::string& path, Manager& manager,
                             Serialize& serialize,
                             std::function<void(Group*, bool)> callBack)
{
    if (ledGroup.find(path) == ledGroup.end())
    {
        ledGroup.emplace(path, std::make_unique<Group>(
                                   phosphor::led::utils::DBusHandler::getBus(),
                                   path.c_str(), manager, serialize, callBack));
    }
}

void CustomDBus::updateAsserted(const std::string& path, bool value)
{
    if (ledGroup.find(path) != ledGroup.end())
    {
        ledGroup.at(path)->updateAsserted(value);
    }
}

} // namespace led
} // namespace phosphor
