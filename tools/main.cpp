#include "clear-psu-fault-leds.hpp"
#include "sync-fault-leds.hpp"
#include "toggle-fault-leds.hpp"

#include <systemd/sd-bus.h>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

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

/**
 * @brief API to check is field mode enabled.
 *
 * @return true, if field mode is enabled. otherwise false.
 */
bool isFieldModeEnabled() noexcept
{
    try
    {
        std::vector<std::string> l_cmdOutput;
        std::array<char, 256> l_buffer;

        std::string l_cmd = "/sbin/fw_printenv fieldmode";

        std::unique_ptr<FILE, int (*)(FILE*)> l_cmdPipe(
            popen(l_cmd.c_str(), "r"), pclose);

        if (!l_cmdPipe)
        {
            throw std::runtime_error("popen failed, error:" +
                                     std::string(strerror(errno)));
        }
        while (fgets(l_buffer.data(), l_buffer.size(), l_cmdPipe.get()) !=
               nullptr)
        {
            l_cmdOutput.emplace_back(l_buffer.data());
        }

        if (l_cmdOutput.size() > 0)
        {
            std::transform(l_cmdOutput[0].begin(), l_cmdOutput[0].end(),
                           l_cmdOutput[0].begin(), [](unsigned char l_char) {
                return std::tolower(l_char);
            });

            // Remove the new line character from the string.
            l_cmdOutput[0].erase(l_cmdOutput[0].length() - 1);

            return l_cmdOutput[0] == "fieldmode=true" ? true : false;
        }
    }
    catch (const std::exception& l_ex)
    {}

    return false;
}

/**
 * @brief An API to create a PEL.
 *
 * This API makes synchronous call to phosphor-logging Create method.
 *
 * @param[in] errorType - Event message name.
 * @param[in] severity - Severity of the event.
 * @param[in] fileName - File name.
 * @param[in] description - Error description.
 */
void createPel(const std::string& errorType, const std::string& severity,
               const std::string& fileName,
               const std::string& description) noexcept
{
    try
    {
        if (errorType.empty() || severity.empty() || fileName.empty() ||
            description.empty())
        {
            throw std::runtime_error(
                "Input(s) passed to create PEL is found empty");
        }

        std::map<std::string, std::string> additionalData{
            {"FILENAME", fileName}, {"DESCRIPTION", description}};

        auto bus = sdbusplus::bus::new_default();
        auto method = bus.new_method_call(
            "xyz.openbmc_project.Logging", "/xyz/openbmc_project/logging",
            "xyz.openbmc_project.Logging.Create", "Create");
        method.append(errorType, severity, additionalData);
        bus.call(method);
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        std::cerr << "PEL creation failed with an error: " +
                         std::string(ex.what())
                  << std::endl;
    }
}

/**
 * @brief API to log a PEL if just not help command.
 *
 * The API logs a PEL when led-tool being used in the system.
 * PEL will be skipped in case help command used.
 *
 * @param[in] argc - Number of arguments recieved.
 * @param[in] argv - Arrguments passed.
 */
void logPelIfNotHelp(int argc, char** argv) noexcept
{
    bool logPel = true;
    std::string cmd{"user entered command: "};

    for (auto count = 0; count < argc; count++)
    {
        cmd += argv[count];

        if (argv[count] == std::string("--help") ||
            argv[count] == std::string("-h") || argc == 1)
        {
            logPel = false;
        }

        cmd += (count < argc - 1) ? " " : "";
    }

    if (logPel)
    {
        createPel("com.ibm.Info.LedTool",
                  "xyz.openbmc_project.Logging.Entry.Level.Informational",
                  __FILE__, cmd);
    }
}

int main(int argc, char** argv)
{
    if (!isFieldModeEnabled())
    {
        std::cerr << "LED tool enabled only in fieldmode." << std::endl;
        return -1;
    }

    logPelIfNotHelp(argc, argv);

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

    auto clearPsuFaultLed = app.add_flag("-c, --clearPsuFaultLed",
                                         "Clears PSU fault LEDs.");

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
        if (*toggleFaultLed)
        {
            toggleFaultLeds(isFunctional);
        }

        if (*clearPsuFaultLed)
        {
            clearPsuFaultLeds();
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
