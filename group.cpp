#include "config.h"

#include "group.hpp"

#include <sdbusplus/message.hpp>

#ifdef IBM_SAI
#include "ibm-sai.hpp"
#endif

namespace phosphor
{
namespace led
{

/** @brief Overloaded Property Setter function */
bool Group::asserted(bool value)
{
    // If the value is already what is before, return right away
    if (value ==
        sdbusplus::xyz::openbmc_project::Led::server::Group::asserted())
    {
        return value;
    }

    if (customCallBack != nullptr)
    {
        // Call the custom callback method
        customCallBack(this, value);

        return sdbusplus::xyz::openbmc_project::Led::server::Group::asserted(
            value);
    }

    // Introducing these to enable gtest.
    Manager::group ledsAssert{};
    Manager::group ledsDeAssert{};

    // Group management is handled by Manager. The populated leds* sets are not
    // really used by production code. They are there to enable gtest for
    // validation.
    auto result = manager.setGroupState(path, value, ledsAssert, ledsDeAssert);

    // Store asserted state
    serialize.storeGroups(path, result);

#ifdef IBM_SAI
    // When setting the associated FRU's operational status for
    // platform and partition SAI, we need to be sure that when
    // the status is being set to good, both platform and partition
    // SAI group objects are de-asserted.
    std::string otherPath;
    if (path == phosphor::led::ibm::PARTITION_SAI)
    {
        otherPath = phosphor::led::ibm::PLATFORM_SAI;
    }
    else if (path == phosphor::led::ibm::PLATFORM_SAI)
    {
        otherPath = phosphor::led::ibm::PARTITION_SAI;
    }

    if (!otherPath.empty())
    {
        if (value || (!value && !manager.isAsserted(otherPath)))
        {
            phosphor::led::ibm::setOperationalStatus(path, !value);
        }
    }
#endif

    // If something does not go right here, then there should be an sdbusplus
    // exception thrown.
    manager.driveLEDs(ledsAssert, ledsDeAssert);

    // Set the base class's asserted to 'true' since the getter
    // operation is handled there.
    return sdbusplus::xyz::openbmc_project::Led::server::Group::asserted(
        result);
}

} // namespace led
} // namespace phosphor
