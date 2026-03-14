#pragma once

#include <map>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>

#include "system_util.hpp"
#include "string_trim.hpp"

class doxy_config
{

public:
    static doxy_config &instance()
    {
        static doxy_config singleton;
        return singleton;
    }

    doxy_config(const doxy_config &) = delete;
    doxy_config &operator=(const doxy_config &) = delete;
    doxy_config(doxy_config &&) = delete;
    doxy_config &operator=(doxy_config &&) = delete;

    void create_doxyfile(std::string doxyfile = "")
    {
        if (doxyfile == "")
            doxyfile = config_file;
        
        if (!std::filesystem::exists(doxyfile))
            system_util::instance().run_command(config_file_cmd);
    }

    bool config_exists(std::string doxyfile = "")
    {
        if (doxyfile == "")
            doxyfile = config_file;

        return std::filesystem::exists(doxyfile);
    }

    size_t load_config(std::string doxyfile = "")
    {
        std::string line;

        if (doxyfile == "")
            doxyfile = config_file;

        std::ifstream hfile(doxyfile);

        clear_config();

        if (!std::filesystem::exists(doxyfile))
        {
            std::cout << "Failed to find " << doxyfile << std::endl;
            std::cout << "Hit any key to exit..." << std::endl;
            getchar();
            exit(1);
        }

        if (hfile.is_open())
        {
            while (std::getline(hfile, line))
            {
                lines.push_back(line);
            }
            hfile.close();
        }
        else
        {
            std::cout << "Failed to open " << doxyfile << std::endl;
            std::cout << "Hit any key to exit..." << std::endl;
            getchar();
            exit(1);
        }

        return lines.size();
    }

    size_t save_config(std::string doxyfile = "")
    {
        size_t line_count = 0;

        if (doxyfile == "")
            doxyfile = config_file;

        std::ofstream conf(doxyfile, std::ofstream::trunc);
        
        if (conf.is_open())
        {
            for (auto &line : lines)
            {
                line_count++;
                conf << line << std::endl;
            }
            conf.flush();
            conf.close();
        }

        return line_count;
    }

    void overwrite_def_values()
    {
        update_value("RECURSIVE", "YES");
        update_value("OUTPUT_DIRECTORY", "output");
        update_value("PROJECT_NUMBER", "1.2.3.4");
        update_value("PROJECT_NAME", "My Project");
        update_value("PROJECT_BRIEF", "Nadav Hugim");
        update_value("EXTRACT_ALL", "YES");
        update_value("SOURCE_BROWSER", "YES");
        update_value("GENERATE_TREEVIEW", "YES");
        update_value("ENUM_VALUES_PER_LINE", "1");
        update_value("GENERATE_LATEX", "NO");
        update_value("MACRO_EXPANSION", "YES");
        update_value("CALL_GRAPH", "YES");
        update_value("CALLER_GRAPH", "YES");
    }

    void clear_config()
    {
        lines.clear();
    }

    void handle_user_input()
    {
        std::string key, value;
        key = "PROJECT_NAME";
        value = get_value(key);
        if(system_util::instance().user_input("PROJECT_NAME", value, true))
            update_value(key, value);

        key = "PROJECT_NUMBER";
        value = get_value(key);
        if(system_util::instance().user_input("PROJECT_NUMBER", value, true))
            update_value(key, value);

        key = "PROJECT_BRIEF";
        value = get_value(key);
        if(system_util::instance().user_input("PROJECT_BRIEF", value, true))
            update_value(key, value);

        key = "OUTPUT_DIRECTORY";
        value = get_value(key);
        if(system_util::instance().user_input("OUTPUT_DIRECTORY", value, true))
            update_value(key, value);

        key = "TAB_SIZE";
        value = get_value(key);
        if(system_util::instance().user_input("TAB_SIZE", value, true))
            update_value(key, value);

        key = "INPUT";
        value = get_value(key);
        if(system_util::instance().user_input("INPUT", value, true))
            update_value(key, value);

        key = "RECURSIVE";
        value = get_value(key);
        if(system_util::instance().user_input("RECURSIVE", value, true))
            update_value(key, value);

        key = "ENUM_VALUES_PER_LINE";
        value = get_value(key);
        if(system_util::instance().user_input("ENUM_VALUES_PER_LINE", value, true))
            update_value(key, value);
    }

    std::string operator[](const std::string &key)
    {
        // search for key in lines vector and return value if found
        for (const auto &line : lines)
        {
            if (line.size() == 0)
                continue;

            if (line[0] == '#')
                continue;

            std::size_t found = line.find_first_of("=");

            if (found != std::string::npos)
            {
                std::string tmpline = line;
                std::string k_sub = tmpline.substr(0, found);
                std::string v_sub = tmpline.substr(found + 1);
                std::string k = string_trim::trim(k_sub);
                std::string v = string_trim::trim(v_sub);

                if (k == key)
                    return v;
            }
        }
        return "";
    }

    std::string get_value(const std::string &key)
    {
        return (*this)[key];
    }

    void update_value(const std::string &key, const std::string &value)
    {
        for (auto &line : lines)
        {
            if (line.size() == 0)
                continue;

            if (line[0] == '#')
                continue;

            std::size_t found = line.find_first_of("=");

            if (found != std::string::npos)
            {
                std::string tmpline = line;
                std::string k_sub = tmpline.substr(0, found);
                std::string k = string_trim::trim(k_sub);

                if (k == key)
                {
                    line = key + " = " + value;
                    break;
                }
            }
        }
    }

    std::string get_doxy_cmd()
    {
        return doxygen_file_cmd;
    }

    void set_config_file(const std::string &file)
    {
        config_file = file;
    }

private:
    doxy_config() = default;
    ~doxy_config() noexcept = default;

    std::vector<std::string> lines;
    std::string config_file = "/tmp/doxymate.cfg";
    const std::string config_file_cmd = "doxygen -g " + config_file + " >> /dev/null";
    const std::string doxygen_file_cmd = "doxygen " + config_file;
};
