#pragma once

#include <string>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <sys/wait.h>

class system_util
{
public:
    static system_util &instance()
    {
        static system_util instance;
        return instance;
    }

    system_util(const system_util &) = delete;
    system_util &operator=(const system_util &) = delete;
    system_util(system_util &&) = delete;
    system_util &operator=(system_util &&) = delete;

    void run_command(const std::string &cmd)
    {
        const int returnCode = std::system(cmd.c_str());

        if (returnCode != 0)
        {
            int exitCode = returnCode;
            if (WIFEXITED(returnCode))
                exitCode = WEXITSTATUS(returnCode);

            std::cout << "Command " << cmd << " fail with error: " << exitCode << std::endl;
            std::cout << "Hit any key to exit..." << std::endl;
            getchar();
            exit(exitCode);
        }
    }

    std::string get_running_directory()
    {
        return std::filesystem::current_path().string();
    }   

    void open_with_default_browser(const std::string &path)
    {
        const std::string absolutePath = std::filesystem::absolute(path).string();
        const std::string quotedPath = "\"" + absolutePath + "\"";

        if (std::getenv("WSL_DISTRO_NAME") != nullptr && command_exists("wslview"))
        {
            if (try_command("wslview " + quotedPath + " >/dev/null 2>&1"))
                return;
        }

        if (command_exists("xdg-open") && try_command("xdg-open " + quotedPath + " >/dev/null 2>&1"))
            return;

        if (command_exists("gio") && try_command("gio open " + quotedPath + " >/dev/null 2>&1"))
            return;

        std::cout << "Failed to open browser automatically for: " << absolutePath << std::endl;
        std::cout << "Install one of: wslu (wslview), xdg-utils (xdg-open), or gio." << std::endl;
    }

    void install_doxygen()
    {
        const std::string doxygen_file = "/usr/bin/doxygen";
        if (!std::filesystem::exists(doxygen_file))
            run_command("sudo apt-get install doxygen");
    }

    void install_graphviz()
    {
        const std::string graphviz_file = "/usr/bin/dot";
        if (!std::filesystem::exists(graphviz_file))
            run_command("sudo apt-get install graphviz");
    }

    void install_figlet()
    {
        const std::string figlet_file = "/usr/bin/figlet";
        if (!std::filesystem::exists(figlet_file))
            run_command("sudo apt-get install figlet");
    }

    bool user_input(const std::string &key, std::string &def_val, bool canbeempty)
    {
        while (true)
        {
            std::string value;

            if (def_val != "")
                std::cout << key << " ? [" << def_val << "] ";
            else
                std::cout << key << " ? ";

            std::getline(std::cin, value, '\n');

            if (value != "")
            {
                std::cout << "updated " << value << std::endl;
                def_val = value;
                return true;
            }

            if (canbeempty)
                break;
        }
        return true;
    }

private:
    system_util() = default;
    ~system_util() = default;

    bool command_exists(const std::string &command)
    {
        return std::system(("command -v " + command + " >/dev/null 2>&1").c_str()) == 0;
    }

    bool try_command(const std::string &command)
    {
        const int returnCode = std::system(command.c_str());
        if (returnCode == 0)
            return true;

        if (WIFEXITED(returnCode) && WEXITSTATUS(returnCode) == 0)
            return true;

        return false;
    }
};
