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
               "of the FRUs accordingly.")
            ->needs(functional);

    CLI11_PARSE(app, argc, argv);

    try
    {
        if (*toggleFaultLed)
        {
            toggleFaultLeds(isFunctional);
        }
    }
    catch (const std::exception& ex)
    {
        std::cout << "Led tool failed with exception: " << ex.what()
                  << std::endl;
    }
    return 0;
}