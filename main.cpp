#include <map>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "string_trim.hpp"
#include "system_util.hpp"
#include "doxy_config.hpp"

doxy_config &gconfig = doxy_config::instance();

void handle_config()
{
    bool configExists = doxy_config::instance().config_exists();

    if (configExists)
    {
        std::cout << "Found existing doxyfile, loading..." << std::endl;
        doxy_config::instance().load_config();
    }
    else
    {
        std::cout << "No doxyfile found, creating default one..." << std::endl;
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

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        std::string configfilename = argv[1];
        std::cout << "Received argument config file path: " << configfilename << std::endl;
        doxy_config::instance().set_config_file(configfilename);
    }
    else
    {
        std::cout << "No config file argument provided, using default: " << doxy_config::instance().get_doxy_cmd() << std::endl;
    }

    std::cout << "Running in directory: " << system_util::instance().get_running_directory() << std::endl;

    handle_install();
    handle_config();

    // banner
    system_util::instance().run_command("figlet DoxyNad from code to documentation");

    // run doxygen & build html
    system_util::instance().run_command(doxy_config::instance().get_doxy_cmd());

    const std::string outputDirectory = doxy_config::instance()["OUTPUT_DIRECTORY"];
    const std::string htmlFile = outputDirectory + "/html/index.html";
    std::string absoluteHtmlFile = std::filesystem::absolute(htmlFile).string();

    if (std::filesystem::exists(absoluteHtmlFile))
    {
        std::cout << "Documentation generated successfully at: " << absoluteHtmlFile << std::endl;
    }
    else
    {
        std::cout << "Failed to generate documentation." << std::endl;
    }
}