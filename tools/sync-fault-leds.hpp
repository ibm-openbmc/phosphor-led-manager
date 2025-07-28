#pragma once
#include "utility.hpp"

/**
 * @brief API to set the asserted property of fault LED of a FRU.
 *
 * @param[in] i_fruName - Name of the FRU.
 * @param[in] i_value - Value to be set.
 *
 * @return On success returns true, false otherwise.
 */
inline bool setFaultLedAssertedProperty(const std::string& i_fruName,
                                        const bool& i_value) noexcept
{
    return utility::setProperty<bool>(
        "xyz.openbmc_project.LED.GroupManager",
        "/xyz/openbmc_project/led/groups/" + i_fruName + "_fault",
        "xyz.openbmc_project.Led.Group", "Asserted", i_value);
}

/**
 * @brief API to set the state property of LED for a given FRU.
 *
 * @param[in] i_fruName - Name of the FRU.
 * @param[in] i_value - Value to be set.
 *
 * @return On success returns true, false otherwise.
 */
inline bool setPhysicalLedStateProperty(const std::string& i_fruName,
                                        const std::string& i_value) noexcept
{
    return utility::setProperty<std::string>(
        "xyz.openbmc_project.LED.Controller.pca955x_" + i_fruName,
        "/xyz/openbmc_project/led/physical/pca955x_" + i_fruName,
        "xyz.openbmc_project.Led.Physical", "State", i_value);
}

/**
 * @brief API to sync asserted and state properties of a fault LED of a FRU.
 *
 * This API syncs asserted and state properties of a fault LED of a FRU
 * according to the functional property of the FRU.
 *
 * @param[in] i_objectPath - Object path of FRU
 *
 * @throw std::runtime_error
 */
void doSyncFaultLed(const std::string& i_objectPath)
{
    try
    {
        constexpr auto MAX_CONFIRMATION_STR_LENGTH{3};
        std::string l_confirmation{};
        std::cout
            << "Don't use this option if the FRU is in identified state. Doing this can create out of sync issue for the LED. Do you really wish to proceed further?[yes/no]:";
        std::cin >> std::setw(MAX_CONFIRMATION_STR_LENGTH) >> l_confirmation;

        if (l_confirmation != "yes")
        {
            return;
        }

        // extract the FRU name from object path
        const std::string& l_fruName =
            sdbusplus::message::object_path(i_objectPath).filename();

        // get the Functional property from PIM
        const auto l_functionalProperty = utility::getProperty(
            "xyz.openbmc_project.Inventory.Manager",
            "/xyz/openbmc_project/inventory" + i_objectPath,
            "xyz.openbmc_project.State.Decorator.OperationalStatus",
            "Functional");

        if (const auto l_functionalPropertyVal =
                std::get_if<bool>(&l_functionalProperty))
        {
            const bool& l_functional = *l_functionalPropertyVal;

            // set asserted property of fault LED
            // asserted property is inverse of functional property
            if (!setFaultLedAssertedProperty(l_fruName, !l_functional))
            {
                throw std::runtime_error("Failed to set asserted property.");
            }

            // set physical LED state
            if (!setPhysicalLedStateProperty(
                    l_fruName,
                    l_functional
                        ? "xyz.openbmc_project.Led.Physical.Action.Off"
                        : "xyz.openbmc_project.Led.Physical.Action.On"))
            {
                throw std::runtime_error("Failed to set physical property.");
            }
        }
        else
        {
            throw std::runtime_error("Failed to get functional property.");
        }
    }
    catch (const std::exception& l_ex)
    {
        throw std::runtime_error(
            "Failed to sync fault LED properties for FRU [" + i_objectPath +
            "]. Error: " + std::string(l_ex.what()));
    }
}
