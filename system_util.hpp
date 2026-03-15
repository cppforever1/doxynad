#pragma once

#include <string>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <sys/wait.h>
#include <spdlog/spdlog.h>
#include <cstdio> // for fdopen, getline, fclose

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

    void run_and_log(const std::string &cmd)
    {
        int out_pipe[2];
        int err_pipe[2];

        if (pipe(out_pipe) == -1)
        {
            spdlog::error("pipe(out_pipe) failed");
            return;
        }

        if (pipe(err_pipe) == -1)
        {
            spdlog::error("pipe(err_pipe) failed");
            close(out_pipe[0]);
            close(out_pipe[1]);
            return;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            spdlog::error("fork() failed");
            close(out_pipe[0]);
            close(out_pipe[1]);
            close(err_pipe[0]);
            close(err_pipe[1]);
            return;
        }

        if (pid == 0)
        {
            // Child
            dup2(out_pipe[1], STDOUT_FILENO);
            dup2(err_pipe[1], STDERR_FILENO);

            close(out_pipe[0]);
            close(out_pipe[1]);
            close(err_pipe[0]);
            close(err_pipe[1]);

            execl("/bin/sh", "sh", "-c", cmd.c_str(), (char *)nullptr);
            _exit(127);
        }

        // Parent
        close(out_pipe[1]);
        close(err_pipe[1]);

        FILE *out_stream = fdopen(out_pipe[0], "r");
        FILE *err_stream = fdopen(err_pipe[0], "r");

        if (!out_stream || !err_stream)
        {
            spdlog::error("fdopen failed");

            if (out_stream)
            {
                fclose(out_stream);
            }
            else
            {
                close(out_pipe[0]);
            }

            if (err_stream)
            {
                fclose(err_stream);
            }
            else
            {
                close(err_pipe[0]);
            }

            int status = 0;
            waitpid(pid, &status, 0);
            return;
        }

        char *line = nullptr;
        size_t cap = 0;
        ssize_t nread = 0;

        while ((nread = getline(&line, &cap, out_stream)) != -1)
        {
            if (nread > 0 && line[nread - 1] == '\n')
                line[nread - 1] = '\0';

            spdlog::trace("{}", line);
        }

        while ((nread = getline(&line, &cap, err_stream)) != -1)
        {
            if (nread > 0 && line[nread - 1] == '\n')
                line[nread - 1] = '\0';
            spdlog::error("{}", line);
        }

        free(line);
        fclose(out_stream); // also closes out_pipe[0]
        fclose(err_stream); // also closes err_pipe[0]

        int status = 0;
        waitpid(pid, &status, 0);
    }

    void run_command(const std::string &cmd)
    {
        const int returnCode = std::system(cmd.c_str());

        if (returnCode != 0)
        {
            int exitCode = returnCode;
            if (WIFEXITED(returnCode))
                exitCode = WEXITSTATUS(returnCode);

            spdlog::error("Command {} failed with error: {}", cmd, exitCode);
            spdlog::info("Hit any key to exit...");
            getchar();
            exit(exitCode);
        }
    }

    std::string get_running_directory()
    {
        return std::filesystem::current_path().string();
    }

    bool test_sudo()
    {
        return std::getenv("SUDO_USER") != nullptr;
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

        spdlog::error("Failed to open browser automatically for: {}", absolutePath);
        spdlog::info("Install one of: wslu (wslview), xdg-utils (xdg-open), or gio.");
    }

    void install_doxygen()
    {
        const std::string doxygen_file = "/usr/bin/doxygen";
        if (!std::filesystem::exists(doxygen_file))
        {
            if (test_sudo())
            {
                run_command("sudo apt-get install doxygen");
            }
            else
            {
                spdlog::error("Doxygen is not installed. Please install it manually or run this program with sudo.");
                exit(1);
            }
        }
    }

    void install_graphviz()
    {
        const std::string graphviz_file = "/usr/bin/dot";
        if (!std::filesystem::exists(graphviz_file))
        {
            if (test_sudo())
            {
                run_command("sudo apt-get install graphviz");
            }
            else
            {
                spdlog::error("Graphviz is not installed. Please install it manually or run this program with sudo.");
                exit(1);
            }
        }
    }

    void install_figlet()
    {
        const std::string figlet_file = "/usr/bin/figlet";
        if (!std::filesystem::exists(figlet_file))
        {
            if (test_sudo())
            {
                run_command("sudo apt-get install figlet");
            }
            else
            {
                spdlog::error("Figlet is not installed. Please install it manually or run this program with sudo.");
                exit(1);
            }
        }
    }

    bool user_input(const std::string &key, std::string &def_val, bool canbeempty)
    {
        while (true)
        {
            std::string value;

            if (def_val != "")
                spdlog::info("{} ? [{}] ", key, def_val);
            else
                spdlog::info("{} ? ", key);

            std::getline(std::cin, value, '\n');

            if (value != "")
            {
                spdlog::info("updated {}", value);
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
        spdlog::info("Checking if command exists: {}", command);
        std::string cmd = "command -v " + command + " >/dev/null 2>&1";

        bool exists = std::system(cmd.c_str()) == 0;
        spdlog::info("Command {} exists: {}", command, exists);

        return exists;
    }

    bool try_command(const std::string &command)
    {
        spdlog::info("Trying command: {}", command);

        const int returnCode = std::system(command.c_str());
        spdlog::info("Command returned code: {}", returnCode);

        if (returnCode == 0)
            return true;

        if (WIFEXITED(returnCode) && WEXITSTATUS(returnCode) == 0)
            return true;

        return false;
    }
};
