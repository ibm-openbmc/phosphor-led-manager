#include "set-guarded-fru-leds.hpp"
#include "set-leds-default-state.hpp"
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

    CLI11_PARSE(app, argc, argv);

    try
    {
        if (*toggleFaultLed)
        {
            toggleFaultLeds(isFunctional);
        }

        if (*setGuardedFruLeds)
        {
            setLEDForGuardedFru();
        }

        if (*defaultLedsSate)
        {
            setLedsDefaultState();
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Led tool failed with exception: " << ex.what()
                  << std::endl;
    }

    return 0;
}
