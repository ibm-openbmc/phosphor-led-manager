#include "clear-psu-fault-leds.hpp"
#include "set-guarded-fru-leds.hpp"
#include "set-leds-default-state.hpp"
#include "sync-fault-leds.hpp"
#include "toggle-fault-leds.hpp"

#include <CLI/CLI.hpp>

int main(int argc, char** argv)
{
    CLI::App app{"led-tool - A tool to perform opeartion(s) over LEDs."};

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
    }
    catch (const std::exception& ex)
    {
        std::cout << "Led tool failed with exception: " << ex.what()
                  << std::endl;
    }

    return 0;
}
