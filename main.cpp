#include <map>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "string_trim.hpp"
#include "system_util.hpp"
#include "doxy_config.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>

void handle_config()
{
    const bool configExists = doxy_config::instance().config_exists();
    const std::string configfile = doxy_config::instance().get_config_file();

    if (configExists)
    {
        // load existing config
        spdlog::info("Found existing doxyfile, loading {}", configfile);
        doxy_config::instance().load_config();
    }
    else
    {
        // create default config
        spdlog::info("No doxyfile found, creating one {}", configfile);
        doxy_config::instance().create_doxyfile();
        doxy_config::instance().load_config();
        doxy_config::instance().overwrite_def_values();
    }

    doxy_config::instance().handle_user_input();
    doxy_config::instance().save_config();
}

void handle_install()
{
    system_util::instance().install_doxygen();
    system_util::instance().install_graphviz();
    system_util::instance().install_figlet();
}

// initialize spdlog with default settings for file and console color logging
void init_logger()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern("%v");

    auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/daily.txt", 0, 0, false, 60);
    file_sink->set_level(spdlog::level::info);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("multi_sink", sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    spdlog::set_default_logger(logger);

    std::string starline(40, '*');
    spdlog::info(" {} DoxyNad started {}", starline, starline);
}


void test_config()
{
        auto header = doxy_config::instance().get_header();
    for (const auto &line : header)
    {
        spdlog::info("{}", line);
    }

    auto sections = doxy_config::instance().get_sections();
    spdlog::info("Config sections:");
    for (const auto &section : sections)
    {
        spdlog::info(" - {}", section);
    }

    auto keys = doxy_config::instance().get_keys_params();
    spdlog::info("Config keys:");
    for (const auto &key : keys)
    {
        spdlog::info(" - {}", key);
    }

    auto help = doxy_config::instance().get_key_help("OUTPUT_DIRECTORY");
    spdlog::info("Help for key OUTPUT_DIRECTORY:");
    for (const auto &line : help)    {
        spdlog::info("{}", line);
    }
}

int main(int argc, char **argv)
{
    init_logger();

    if (argc == 2)
    {
        // set config file path from argument
        std::string configfilename = argv[1];
        spdlog::info("Received argument config file path: {}", configfilename);
        doxy_config::instance().set_config_file(configfilename);
    }
    else
    {
        // no config file argument provided, using default
        spdlog::info("No config file argument provided, using default: {}", doxy_config::instance().get_config_file());
    }

    spdlog::info("Running in directory: {}", system_util::instance().get_running_directory());

    // install dependencies
    handle_install();

    // handle doxyfile config
    handle_config();

    //test_config();

    // banner
    system_util::instance().run_command("figlet DoxyNad from code to documentation");

    // run doxygen & build html
    system_util::instance().run_and_log(doxy_config::instance().get_doxy_cmd());

    const std::string outputDirectory = doxy_config::instance()["OUTPUT_DIRECTORY"];
    const std::string htmlFile = outputDirectory + "/html/index.html";
    std::string absoluteHtmlFile = std::filesystem::absolute(htmlFile).string();

    if (std::filesystem::exists(absoluteHtmlFile))
    {
        spdlog::info("Documentation generated successfully at: {}", absoluteHtmlFile);
    }
    else
    {
        spdlog::error("Failed to generate documentation.");
    }
}