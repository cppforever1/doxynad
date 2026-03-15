#pragma once

#include <map>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>

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
        {
            system_util::instance().run_command(config_file_cmd);

            if (!std::filesystem::exists(doxyfile))
            {
                spdlog::error("Failed to create doxyfile at: {}", doxyfile);
                spdlog::info("Hit any key to exit...");
                getchar();
                exit(1);
            }
        }
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
            spdlog::error("Failed to find {}", doxyfile);
            spdlog::info("Hit any key to exit...");
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
            spdlog::error("Failed to open {}", doxyfile);
            spdlog::info("Hit any key to exit...");
            getchar();
            exit(1);
        }

        spdlog::info("Loaded config file: {} with {} lines", doxyfile, lines.size());

        return lines.size();
    }

    std::vector<std::string> get_header() const
    {
        std::string templine;
        std::vector<std::string> header;

        for (const auto &line : lines)
        {
            if (line.find_first_of("#-------------") != std::string::npos)
                break;

            if (line.size() == 0)
                continue;

            templine = line.substr(0);
            templine = string_trim::trim(templine);
            header.push_back(templine);
        }
        return header;
    }

    std::vector<std::string> get_sections() const
    {
        std::string templine;
        std::vector<std::string> sections;

        for (size_t idx = 0; idx < lines.size(); idx++)
        {
            const auto &line = lines[idx];

            if (line.size() == 0)
                continue;

            if (line.find_first_of("#-------------") != std::string::npos)
            {
                templine = lines[++idx].substr(0);
                templine = string_trim::trim(templine);
                sections.push_back(templine);
                idx++; // skip next line which is ------------- separator
            }
        }
        return sections;
    }

    std::vector<std::string> get_keys_params() const
    {
        std::string templine;
        std::vector<std::string> keys;

        for (const auto &line : lines)
        {
            if (line.size() == 0)
                continue;

            if (line[0] == '#')
                continue;

            if (line.find_first_of("=") != std::string::npos)
            {
                templine = line.substr(0, line.find_first_of("="));
                templine = string_trim::trim(templine);
                keys.push_back(templine);
            }
        }
        return keys;
    }

    std::vector<std::string> get_key_help(const std::string &key) const
    {
        std::vector<std::string> help;

        for (size_t idx = lines.size() -1; idx > 0; idx--)
        {
            const auto &line = lines[idx];

            if (line.size() == 0)
                continue;

            if (line.find(key) != std::string::npos)
            {
                for(size_t subidx = idx - 1; subidx > 0; subidx--)
                {
                    const auto &hline = lines[subidx];

                    if (hline.size() == 0)
                        continue;

                    std::string templine = hline.substr(0);
                    templine = string_trim::trim(templine);
                    help.insert(help.begin(), templine);

                    if (line.find("-") != std::string::npos)
                    {
                        break;
                    }
                }
            }
        }

        return help;
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
        else
        {
            spdlog::error("Failed to save {}", doxyfile);
            spdlog::info("Hit any key to exit...");
            getchar();
            exit(1);
        }

        spdlog::info("Saved config file: {} with {} lines", doxyfile, line_count);

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
        update_value("ALLOW_UNICODE_NAMES", "YES");
        update_value("INLINE_INHERITED_MEMB", "YES");
        update_value("REFERENCED_BY_RELATION", "YES");
        update_value("REFERENCES_RELATION", "YES");
        update_value("CLANG_ASSISTED_PARSING", "YES");
        update_value("HTML_DYNAMIC_SECTIONS", "YES");
        update_value("EXCLUDE", "out output build");
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
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "PROJECT_NUMBER";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "PROJECT_BRIEF";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "OUTPUT_DIRECTORY";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "TAB_SIZE";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "INPUT";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "RECURSIVE";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "ENUM_VALUES_PER_LINE";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "INLINE_INHERITED_MEMB";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "REFERENCED_BY_RELATION";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "REFERENCES_RELATION";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "CLANG_ASSISTED_PARSING";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "HTML_DYNAMIC_SECTIONS";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
            update_value(key, value);

        key = "EXCLUDE";
        value = get_value(key);
        if (system_util::instance().user_input(key, value, true))
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
        for (std::string &line : lines)
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

    std::string get_doxy_cmd() const
    {
        return doxygen_file_cmd;
    }

    void set_config_file(const std::string &file)
    {
        config_file = file;
    }

    std::string get_config_file() const
    {
        return config_file;
    }

private:
    doxy_config() = default;
    ~doxy_config() noexcept = default;

    std::vector<std::string> lines;
    std::string config_file = "/tmp/doxymate.cfg";
    const std::string config_file_cmd = "doxygen -g " + config_file + " >> /dev/null";
    const std::string doxygen_file_cmd = "doxygen " + config_file;
};
