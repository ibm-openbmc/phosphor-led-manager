#include "config.h"

#include "group.hpp"
#ifdef LED_USE_JSON
#include "json-parser.hpp"
#else
#include "led-gen.hpp"
#endif
#include "ledlayout.hpp"
#include "manager.hpp"
#include "serialize.hpp"
#include "utils.hpp"
#ifdef USE_LAMP_TEST
#include "lamptest.hpp"
#endif

#include <CLI/CLI.hpp>
#include <sdeventplus/event.hpp>

#include <iostream>

int main(int argc, char** argv)
{
    CLI::App app("phosphor-led-manager");

#ifdef LED_USE_JSON
    std::string configFile{};
    app.add_option("-c,--config", configFile, "Path to JSON config");
#endif

    CLI11_PARSE(app, argc, argv);

    // Get a default event loop
    auto event = sdeventplus::Event::get_default();

    /** @brief Dbus constructs used by LED Group manager */
    auto& bus = phosphor::led::utils::DBusHandler::getBus();

#ifdef LED_USE_JSON
    auto systemLedMap = getSystemLedMap(configFile);
#endif

    /** @brief Group manager object */
    phosphor::led::Manager manager(bus, systemLedMap);

    /** @brief sd_bus object manager */
    sdbusplus::server::manager::manager objManager(bus, OBJPATH);

#ifdef USE_LAMP_TEST
    if (std::filesystem::exists(LAMP_TEST_INDICATOR_FILE))
    {
        // we need to off all the LEDs.
        phosphor::led::utils::DBusHandler dBusHandler;
        std::vector<std::string> pysicalLedPaths = dBusHandler.getSubTreePaths(
            phosphor::led::PHY_LED_PATH, phosphor::led::PHY_LED_IFACE);

        for (const auto& path : pysicalLedPaths)
        {
            manager.drivePhysicalLED(path, phosphor::led::Layout::Action::Off,
                                     0, 0);
        }

        // Also remove the lamp test on indicator file.
        if (!std::filesystem::remove(LAMP_TEST_INDICATOR_FILE))
        {
            lg2::error("error removing file after lamp test is over");
        }
    }
#endif

    /** @brief vector of led groups */
    std::vector<std::unique_ptr<phosphor::led::Group>> groups;

    /** @brief store and re-store Group */
    phosphor::led::Serialize serialize(SAVED_GROUPS_FILE);

#ifdef USE_LAMP_TEST
    phosphor::led::LampTest lampTest(event, manager);

    groups.emplace_back(std::make_unique<phosphor::led::Group>(
        bus, LAMP_TEST_OBJECT, manager, serialize,
        std::bind(std::mem_fn(&phosphor::led::LampTest::requestHandler),
                  &lampTest, std::placeholders::_1, std::placeholders::_2)));

    // Register a lamp test method in the manager class, and call this method
    // when the lamp test is started
    manager.setLampTestCallBack(
        std::bind(std::mem_fn(&phosphor::led::LampTest::processLEDUpdates),
                  &lampTest, std::placeholders::_1, std::placeholders::_2));
#endif

    /** Now create so many dbus objects as there are groups */
    for (auto& grp : systemLedMap)
    {
        groups.emplace_back(std::make_unique<phosphor::led::Group>(
            bus, grp.first, manager, serialize));
    }

    // Attach the bus to sd_event to service user requests
    bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);

    /** @brief Claim the bus */
    bus.request_name(BUSNAME);
    event.loop();

    return 0;
}
