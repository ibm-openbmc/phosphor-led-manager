#include "clear-psu-fault-leds.hpp"
#include "set-guarded-fru-leds.hpp"
#include "set-leds-default-state.hpp"
#include "sync-fault-leds.hpp"
#include "toggle-fault-leds.hpp"

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

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

int main(int argc, char** argv)
{
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

    CLI11_PARSE(app, argc, argv);

    try
    {
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
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Led tool failed with exception: " << ex.what()
                  << std::endl;
    }

    return 0;
}
