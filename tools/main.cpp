#include "clear-psu-fault-leds.hpp"
#include "set-guarded-fru-leds.hpp"
#include "set-leds-default-state.hpp"
#include "sync-fault-leds.hpp"
#include "toggle-fault-leds.hpp"
#include "utility.hpp"

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>

/**
 * @brief An API to dump asserted LED paths to console.
 *
 * @returns 0 for success, -1 for failure.
 */
int dumpAssertedLedObjectPaths()
{
    const std::string savedGroupsPath =
        "/var/lib/phosphor-led-manager/savedGroups";
    try
    {
        if (std::filesystem::exists(savedGroupsPath))
        {
            std::ifstream ledsFilePath(savedGroupsPath);

            if (!ledsFilePath)
            {
                throw std::runtime_error(
                    "Unable to open saved groups file. Can't dump LED path(s).");
            }

            const nlohmann::json parsedLedFile =
                nlohmann::json::parse(ledsFilePath);

            for (const auto& ledPath : parsedLedFile["value0"])
            {
                std::cout << ledPath << std::endl;
            }
        }
        else
        {
            throw std::runtime_error(
                "Saved groups file is not present. Can't dump LED path(s).");
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error while trying to dump asserted led paths. Error : "
                  << ex.what() << std::endl;

        return -1;
    }

    return 0;
}

/**
 * @brief An API to dump LED path with inventory path.
 *
 * This API dumps lED path with its associated inventory paths on the console.
 *
 * @returns 0 for success, -1 for failure.
 */
int dumpLedPathWithInventoryPath()
{
    try
    {
        std::cout << std::setfill('=') << std::setw(150) << "" << std::endl;

        std::cout << std::left << std::setw(70) << std::setfill(' ')
                  << "LED path"
                  << " | " << std::left << std::setw(80) << std::setfill(' ')
                  << "Inventory Path" << std::endl;

        std::cout << std::setfill('=') << std::setw(150) << "" << std::endl;

        auto objectSubTreeMap = utility::getObjectSubtreeForInterfaces(
            "/xyz/openbmc_project/inventory/system", 0,
            {"xyz.openbmc_project.Association.Definitions"});

        for (const auto& objectInterfaceMap : objectSubTreeMap)
        {
            const std::string& inventoryPath = objectInterfaceMap.first;

            auto faultLedPath = utility::GetAssociatedSubTreePaths(
                std::string(inventoryPath + "/fault_identifying"),
                std::string("/"), 0, {});

            auto identifyLedPath = utility::GetAssociatedSubTreePaths(
                std::string(inventoryPath + "/identifying"), std::string("/"),
                0, {});

            for (const auto& ledPath : faultLedPath)
            {
                std::cout << std::left << std::setw(70) << std::setfill(' ')
                          << ledPath << " | " << std::left << std::setw(80)
                          << std::setfill(' ') << inventoryPath << std::endl;

                std::cout << std::setfill('-') << std::setw(150) << ""
                          << std::endl;
            }

            for (const auto& ledPath : identifyLedPath)
            {
                std::cout << std::left << std::setw(70) << std::setfill(' ')
                          << ledPath << " | " << std::left << std::setw(80)
                          << std::setfill(' ') << inventoryPath << std::endl;

                std::cout << std::setfill('-') << std::setw(150) << ""
                          << std::endl;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Error while dumping LED paths with inventory paths to the console. Error : "
            << e.what() << std::endl;

        return -1;
    }

    return 0;
}

int main(int argc, char** argv)
{
    const bool fieldmodeEnabled = utility::isFieldModeEnabled();
    const bool isSystemdService = utility::runningAsSystemDService();

    CLI::App app{"led-tool - A tool to perform operation(s) over LEDs."};

    bool isFunctional = true;
    auto functional = app.add_option(
        "-f, --functional", isFunctional,
        "True/False depending upon the functional status of FRU.");

    auto toggleFaultLed =
        app.add_flag(
               "-t, --toggleFaultLeds",
               "Toggles asserted property for fault LEDs according to the "
               "functional status passed. Also updates functional property "
               "of the FRUs hosted under PIM accordingly.")
            ->needs(functional);

    auto setGuardedFruLeds = app.add_flag("-s, --setGuardedFruLeds",
                                          "Set LEDs for guarded FRUs.");

    auto defaultLedsSate = app.add_flag("-d, --defaultLedsState",
                                        "Sets power, enclosure fault, SAI and "
                                        "enclosure identify to default state.");

    auto clearPsuFaultLed = app.add_flag("-c, --clearPsuFaultLed",
                                         "Clears PSU fault LEDs.");

    auto overrideChassisOnCheckOption = app.add_flag(
        "-x, --overrideChassisOnCheck", "Flag to override chassis on check.");

    std::string objectPath;
    auto objectPathOption = app.add_option("-o, --object", objectPath,
                                           "Object path of the FRU.");
    auto syncFaultLed =
        app.add_flag("-q, --syncFaultLed",
                     "Syncs fault LEDs to functional state of the FRU.")
            ->needs(objectPathOption);

    auto dumpLedObjectPaths = app.add_flag(
        "-D, --dumpLedObjectPaths", "Dumps LED object paths on console.");

    auto dumpAssertedLeds =
        app.add_flag("-a, --dumpAssertedLeds",
                     "Dumps asserted LED object paths to the console.")
            ->needs(dumpLedObjectPaths);

    auto dumpInventoryPathWithLedPath =
        app.add_flag(
               "-i, --dumpInventoryPathWithLedPath",
               "Dumps LED path with its associated inventory path to the console.")
            ->needs(dumpLedObjectPaths);

    CLI11_PARSE(app, argc, argv);

    try
    {
        if (!isSystemdService && !fieldmodeEnabled &&
            (!toggleFaultLed->empty() || !setGuardedFruLeds->empty() ||
             !defaultLedsSate->empty() || !clearPsuFaultLed->empty() ||
             !syncFaultLed->empty()))
        {
            std::cerr << "This option enabled only in field mode." << std::endl;
            return -1;
        }

        if (*toggleFaultLed)
        {
            toggleFaultLeds(isFunctional,
                            !overrideChassisOnCheckOption->empty());
        }

        if (*setGuardedFruLeds)
        {
            setLEDForGuardedFru(!overrideChassisOnCheckOption->empty());
        }

        if (*defaultLedsSate)
        {
            setLedsDefaultState(!overrideChassisOnCheckOption->empty());
        }

        if (*clearPsuFaultLed)
        {
            clearPsuFaultLeds(!overrideChassisOnCheckOption->empty());
        }

        if (!syncFaultLed->empty())
        {
            doSyncFaultLed(objectPath);
        }

        if (!dumpLedObjectPaths->empty())
        {
            if (!dumpAssertedLeds->empty())
            {
                return dumpAssertedLedObjectPaths();
            }

            if (!dumpInventoryPathWithLedPath->empty())
            {
                return dumpLedPathWithInventoryPath();
            }
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Led tool failed with exception: " << ex.what()
                  << std::endl;
    }

    return 0;
}
