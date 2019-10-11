// SPDX-License-Identifier: GPL-3.0-only

#include "file.hpp"

#include <cstdio>
#include <filesystem>
#include <cstring>

namespace Invader::File {
    std::optional<std::vector<std::byte>> open_file(const char *path) {
        // Get the file
        std::filesystem::path file_path(path);

        // Make sure we're dealing with a file we can open
        if(std::filesystem::is_regular_file(file_path)) {
            return std::nullopt;
        }

        // Get the size
        auto sizeb = std::filesystem::file_size(file_path);

        // Get the size and make sure we can use it
        if(sizeb > SIZE_MAX) {
            return std::nullopt;
        }
        auto size = static_cast<std::size_t>(sizeb);

        // Attempt to open it
        std::FILE *file = std::fopen(path, "rb");
        if(!file) {
            return std::nullopt;
        }

        // Read it
        std::vector<std::byte> file_data(static_cast<std::size_t>(size));
        if(std::fread(file_data.data(), size, 1, file) != 1) {
            std::fclose(file);
            return std::nullopt;
        }

        // Return what we got
        std::fclose(file);
        return file_data;
    }

    bool save_file(const char *path, const std::vector<std::byte> &data) {
        // Open the file
        std::FILE *f = std::fopen(path, "wb");
        if(!f) {
            return false;
        }

        // Write
        if(std::fwrite(data.data(), data.size(), 1, f) != 1) {
            std::fclose(f);
            return false;
        }

        std::fclose(f);
        return true;
    }

    std::optional<std::string> tag_path_to_file_path(const std::string &tag_path, const std::vector<std::string> &tags, bool must_exist) {
        // if it's an absolute path, we can't do anything about this
        std::filesystem::path tag_path_path(tag_path);
        if(tag_path_path.is_absolute()) {
            return std::nullopt;
        }

        for(auto &tags_directory_string : tags) {
            std::filesystem::path tags_directory(tags_directory_string);
            auto file_path = tags_directory / tag_path_path;
            if(!must_exist || std::filesystem::exists(file_path)) {
                return file_path.string();
            }
        }
        return std::nullopt;
    }

    std::optional<std::string> file_path_to_tag_path(const std::string &file_path, const std::vector<std::string> &tags, bool must_exist) {
        // Get the absolute file path. It doesn't matter if it exists.
        auto absolute_file_path = std::filesystem::absolute(file_path);
        if(must_exist && !std::filesystem::exists(absolute_file_path)) {
            return std::nullopt;
        }
        auto absolute_file_path_str = absolute_file_path.string();

        for(auto &tags_directory_string : tags) {
            // Get the absolute path
            auto tags_directory = std::filesystem::absolute(tags_directory_string);

            // Go back until we get something that's the same
            auto path_check = absolute_file_path;
            while(path_check.has_parent_path()) {
                path_check = path_check.parent_path();
                if(path_check == tags_directory) {
                    auto path_check_str = path_check.string();
                    auto path_check_size = path_check_str.size();
                    auto converted_path = absolute_file_path_str.substr(path_check_size + 1, absolute_file_path_str.size() - path_check_size - 1);
                    return converted_path;
                }
            }
        }
        return std::nullopt;
    }
}