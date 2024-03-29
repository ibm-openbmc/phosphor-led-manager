#include "config.h"

#include "manager.hpp"

#include <phosphor-logging/lg2.hpp>
#include <sdbusplus/exception.hpp>
#include <xyz/openbmc_project/Led/Physical/server.hpp>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
namespace phosphor
{
namespace led
{

// Assert -or- De-assert
bool Manager::setGroupState(const std::string& path, bool assert,
                            group& ledsAssert, group& ledsDeAssert)
{
    if (assert)
    {
        assertedGroups.insert(&ledMap.at(path));
    }
    else
    {
        if (assertedGroups.contains(&ledMap.at(path)))
        {
            assertedGroups.erase(&ledMap.at(path));
        }
    }

    // This will contain the union of what's already in the asserted group
    group desiredState{};
    for (const auto& grp : assertedGroups)
    {
        desiredState.insert(grp->cbegin(), grp->cend());
    }

    // Find difference between Combined and Desired to identify
    // which LEDs are getting altered
    group transient{};
    std::set_difference(combinedState.begin(), combinedState.end(),
                        desiredState.begin(), desiredState.end(),
                        std::inserter(transient, transient.begin()), ledComp);
    if (transient.size())
    {
        // Find common LEDs between transient and Desired to know if some LEDs
        // are changing state and not really getting DeAsserted
        group ledsTransient{};
        std::set_intersection(
            transient.begin(), transient.end(), desiredState.begin(),
            desiredState.end(),
            std::inserter(ledsTransient, ledsTransient.begin()), ledLess);

        // Find difference between above 2 to identify those LEDs which are
        // really getting DeAsserted
        std::set_difference(transient.begin(), transient.end(),
                            ledsTransient.begin(), ledsTransient.end(),
                            std::inserter(ledsDeAssert, ledsDeAssert.begin()),
                            ledLess);

        // Remove the elements from Current that are being DeAsserted.
        if (ledsDeAssert.size())
        {
            // Power off LEDs that are to be really DeAsserted
            for (auto& it : ledsDeAssert)
            {
                // Update LEDs in "physically asserted" set by removing those
                // LEDs which are De-Asserted
                auto found = currentState.find(it);
                if (found != currentState.end())
                {
                    currentState.erase(found);
                }
            }
        }
    }

    // Now LEDs that are to be Asserted. These could either be fresh asserts
    // -or- change between [On]<-->[Blink]
    group temp{};
    std::unique_copy(desiredState.begin(), desiredState.end(),
                     std::inserter(temp, temp.begin()), ledEqual);
    if (temp.size())
    {
        // Find difference between [desired to be Asserted] and those LEDs
        // that are physically asserted currently.
        std::set_difference(
            temp.begin(), temp.end(), currentState.begin(), currentState.end(),
            std::inserter(ledsAssert, ledsAssert.begin()), ledComp);
    }

    // Update the current actual and desired(the virtual actual)
    currentState = std::move(temp);
    combinedState = std::move(desiredState);

    // If we survive, then set the state accordingly.
    return assert;
}

void Manager::setLampTestCallBack(
    std::function<bool(group& ledsAssert, group& ledsDeAssert)> callBack)
{
    lampTestCallBack = callBack;
}

/** @brief Run through the map and apply action on the LEDs */
void Manager::driveLEDs(group& ledsAssert, group& ledsDeAssert)
{
#ifdef USE_LAMP_TEST
    // Use the lampTestCallBack method and trigger the callback method in the
    // lamp test(processLEDUpdates), in this way, all lamp test operations
    // are performed in the lamp test class.
    if (lampTestCallBack(ledsAssert, ledsDeAssert))
    {
        return;
    }
#endif
    group newReqChangedLeds;
    std::vector<std::pair<group&, group&>> actionsVec = {
        {reqLedsAssert, ledsAssert}, {reqLedsDeAssert, ledsDeAssert}};

    timer.setEnabled(false);
    std::set_union(ledsAssert.begin(), ledsAssert.end(), ledsDeAssert.begin(),
                   ledsDeAssert.end(),
                   std::inserter(newReqChangedLeds, newReqChangedLeds.begin()),
                   ledLess);

    // prepare reqLedsAssert & reqLedsDeAssert
    for (auto pair : actionsVec)
    {
        group tmpSet;

        // Discard current required LED actions, if these LEDs have new actions
        // in newReqChangedLeds.
        std::set_difference(pair.first.begin(), pair.first.end(),
                            newReqChangedLeds.begin(), newReqChangedLeds.end(),
                            std::inserter(tmpSet, tmpSet.begin()), ledLess);

        // Union the remaining LED actions with new LED actions.
        pair.first.clear();
        std::set_union(tmpSet.begin(), tmpSet.end(), pair.second.begin(),
                       pair.second.end(),
                       std::inserter(pair.first, pair.first.begin()), ledLess);
    }

    driveLedsHandler();
    return;
}

/**
 * @brief Checks if sysfs present for the given PSU
 *
 * @param[in] phyLedPath - PSU LED object path.
 *
 * @return true if given psu led object is valid, false otherwise.
 */
static bool isSysfsPresentForPSU(const std::string& phyLedPath)
{
    // Extract the group name from physical led object path (object path like,
    // /xyz/openbmc_project/led/physical/cffps1_68)
    std::string ledGroup = std::filesystem::path(phyLedPath).filename();
    std::replace(ledGroup.begin(), ledGroup.end(), '_', '-');

    // Form the sysfs path for the given led group
    std::filesystem::path sysfsPath(std::string("/sys/class/leds/" + ledGroup));

    // Check for the sysfs existence
    if (std::filesystem::exists(sysfsPath))
    {
        return true;
    }

    return false;
}

// Calls into driving physical LED post choosing the action
int Manager::drivePhysicalLED(const std::string& objPath, Layout::Action action,
                              uint8_t dutyOn, const uint16_t period)
{
    try
    {
        // If Blink, set its property
        if (action == Layout::Action::Blink)
        {
            PropertyValue dutyOnValue{dutyOn};
            PropertyValue periodValue{period};

            dBusHandler.setProperty(objPath, PHY_LED_IFACE, "DutyOn",
                                    dutyOnValue);
            dBusHandler.setProperty(objPath, PHY_LED_IFACE, "Period",
                                    periodValue);
        }

        PropertyValue actionValue{getPhysicalAction(action)};
        dBusHandler.setProperty(objPath, PHY_LED_IFACE, "State", actionValue);
    }
    catch (const std::exception& e)
    {
        // For PSU, if the given driver is not present in sysfs path and
        // set-property call fails, do not log error.
        if ((objPath.find("cffps") != std::string::npos) &&
            (!isSysfsPresentForPSU(objPath)))
        {
            return -1;
        }

        lg2::error(
            "Error setting property for physical LED, ERROR = {ERROR}, OBJECT_PATH = {PATH}",
            "ERROR", e, "PATH", objPath);

        return -1;
    }

    return 0;
}

/** @brief Returns action string based on enum */
std::string Manager::getPhysicalAction(Layout::Action action)
{
    namespace server = sdbusplus::xyz::openbmc_project::Led::server;

    // TODO: openbmc/phosphor-led-manager#5
    //    Somehow need to use the generated Action enum than giving one
    //    in ledlayout.
    if (action == Layout::Action::On)
    {
        return server::convertForMessage(server::Physical::Action::On);
    }
    else if (action == Layout::Action::Blink)
    {
        return server::convertForMessage(server::Physical::Action::Blink);
    }
    else
    {
        return server::convertForMessage(server::Physical::Action::Off);
    }
}

void Manager::driveLedsHandler(void)
{
    group failedLedsAssert;
    group failedLedsDeAssert;

    // This order of LED operation is important.
    if (reqLedsDeAssert.size())
    {
        for (const auto& it : reqLedsDeAssert)
        {
            std::string objPath = std::string(PHY_LED_PATH) + it.name;
            lg2::debug("De-Asserting LED, NAME = {NAME}", "NAME", it.name);
            if (drivePhysicalLED(objPath, Layout::Action::Off, it.dutyOn,
                                 it.period))
            {
                failedLedsDeAssert.insert(it);
            }
        }
    }

    if (reqLedsAssert.size())
    {
        for (const auto& it : reqLedsAssert)
        {
            std::string objPath = std::string(PHY_LED_PATH) + it.name;
            lg2::debug("Asserting LED, NAME = {NAME}", "NAME", it.name);
            if (drivePhysicalLED(objPath, it.action, it.dutyOn, it.period))
            {
                failedLedsAssert.insert(it);
            }
        }
    }

    reqLedsAssert = failedLedsAssert;
    reqLedsDeAssert = failedLedsDeAssert;

    if (reqLedsDeAssert.empty() && reqLedsAssert.empty())
    {
        timer.setEnabled(false);
    }
    else
    {
        timer.restartOnce(std::chrono::seconds(1));
    }

    return;
}
} // namespace led
} // namespace phosphor
