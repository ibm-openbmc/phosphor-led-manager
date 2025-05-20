#pragma once
#include "utility.hpp"

/** @brief API to sync asserted and state properties of a fault LED of a FRU.
 *
 * This API syncs asserted and state properties of a fault LED of a FRU
 * according to the functional property of the FRU.
 *
 * @param[in] i_objectPath - Object path of FRU
 *
 * @throw std::runtime_error
 */
void doSyncFaultLed([[maybe_unused]] const std::string& i_objectPath)
{
    // TODO:
    // 1. For given object path, get the Functional property from PIM
    // 2. Construct associated fault LED object path and get the Asserted
    //    property from LED.Group
    //    If Asserted property is incorrect, update it.
    // 3. Construct associated physical LED object path and get the State
    //    property from LED.Controller.pca955x_* service.
    //    If State property is incorrect, update it.
}
