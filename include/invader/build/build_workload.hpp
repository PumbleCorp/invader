// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__BUILD__BUILD_WORKLOAD_HPP
#define INVADER__BUILD__BUILD_WORKLOAD_HPP

#include <vector>
#include <optional>
#include <string>
#include <filesystem>
#include "../hek/map.hpp"
#include "../resource/resource_map.hpp"
#include "../tag/compiled_tag.hpp"

namespace Invader {
    /**
     * This class is used for building cache files through the compile_map static function.
     */
    class BuildWorkload {
    public:
        /**
         * Compile a map
         * @param scenario          scenario tag to use
         * @param tags_directories  tags directories to use
         * @param engine_target     target a specific engine
         * @param maps_directory    maps directory to use; ignored if building a Dark Circlet map
         * @param with_index        tag index to use
         * @param no_external_tags  do not use cached tags; ignored if building a Dark Circlet map
         * @param always_index_tags always use cached tags; ignored if building a Dark Circlet map
         * @param verbose           output non-error messages to console
         * @param forge_crc         forge the CRC32 of the map
         * @param tag_data_pointer  address the tag data will be loaded to
         */
        static std::vector<std::byte> compile_map(
            const char *scenario,
            const std::vector<std::string> &tags_directories,
            HEK::CacheFileEngine engine_target = HEK::CacheFileEngine::CACHE_FILE_DARK_CIRCLET,
            std::string maps_directory = std::string(),
            const std::vector<std::tuple<HEK::TagClassInt, std::string>> &with_index = std::vector<std::tuple<Invader::HEK::TagClassInt, std::string>>(),
            bool no_external_tags = false,
            bool always_index_tags = false,
            bool verbose = false,
            std::optional<std::uint32_t> forge_crc = std::nullopt,
            std::optional<std::uint32_t> tag_data_address = std::nullopt
        );

    private:
        /**
         * Scenario tag to use
         */
        std::string scenario;

        /**
         * Scenario base name
         */
        std::string scenario_base_name;

        /**
         * Index of the scenario tag to use
         */
        std::size_t scenario_index;

        /**
         * Array of tag directories to use
         */
        std::vector<std::filesystem::path> tags_directories;

        /**
         * Cache file engine target
         */
        HEK::CacheFileEngine engine_target;

        /**
         * Cache file type
         */
        HEK::CacheFileType cache_file_type = HEK::CacheFileType::CACHE_FILE_MULTIPLAYER;

        /**
         * Always index tags when possible
         */
        bool always_index_tags;

        /**
         * Maps directory to use
         */
        std::string maps_directory;

        /**
         * Array of compiled tags
         */
        std::vector<std::unique_ptr<CompiledTag>> compiled_tags;

        /**
         * Number of tags present
         */
        std::size_t tag_count;

        /**
         * Output messages
         */
        bool verbose;

        /**
         * Tag data address to use
         */
        std::uint32_t tag_data_address = HEK::CACHE_FILE_PC_BASE_MEMORY_ADDRESS;

        /**
         * Bitmaps.map assets
         */
        std::vector<Resource> bitmaps;

        /**
         * Sounds.map assets
         */
        std::vector<Resource> sounds;

        /**
         * Sounds.map assets
         */
        std::vector<Resource> loc;

        /**
         * Tag buffer for compiling tags
         */
        std::vector<std::byte> tag_buffer;

        /**
         * Compile and load all required tags
         */
        void load_required_tags();

        /**
         * Modify the ting tag volume depending on the target engine
         */
        void modify_ting_tag();

        /**
         * Compile a tag recursively if tag is not present
         * @param  path          path of tag
         * @param  tag_class_int class of tag
         * @return               tag index of compiled tag
         */
        std::size_t compile_tag_recursively(const char *path, HEK::TagClassInt tag_class_int);

        /**
         * Build a cache file
         * @return  cache file data
         */
        std::vector<std::byte> build_cache_file();

        /**
         * Index tags that can be indexed
         * @param   count               number of tags indexed
         * @param   asset_data_removed  asset data removed
         * @param   potential           potential number of tags that can be indexed
         * @return  tag data removed
         */
        std::size_t index_tags(std::size_t &count, std::size_t &asset_data_removed, std::size_t &potential) noexcept;

        /**
         * Find the external resource offsets for tags (retail and demo)
         * @param   count               number of tags made external
         * @param   asset_data_removed  asset data removed
         * @param   partial             number of partial tags found
         * @param   potential           potential number of tags that can be cached
         */
        void find_external_resource_offsets(std::size_t &count, std::size_t &asset_data_removed, std::size_t &partial, std::size_t &potential) noexcept;

        /**
         * Get the scenario base name
         */
        void get_scenario_base_name();

        /**
         * Populate the tag array
         * @param tag_data the tag data
         */
        void populate_tag_array(std::vector<std::byte> &tag_data);

        /**
         * Append all tag data to the file
         * @param tag_data tag data to append
         * @param file     file to append tag data to
         */
        void add_tag_data(std::vector<std::byte> &tag_data, std::vector<std::byte> &file);

        /**
         * Append tag_data with data from tag
         * @param  tag_data  tag data to append
         * @param  tag_array pointer to tag array
         * @param  tag       tag index to add
         * @return           offset of tag data
         */
        std::size_t add_tag_data_for_tag(std::vector<std::byte> &tag_data, void *tag_array, std::size_t tag);

        /**
         * Get all bitmap and sound data
         * @param file     file to append data to
         * @param tag_data tag data to use
         */
        void add_bitmap_and_sound_data(std::vector<std::byte> &file, std::vector<std::byte> &tag_data);

        /**
         * Get all model data
         * @param vertices vertices to add
         * @param indices  triangles to add
         * @param tag_data tag data to use
         */
        void add_model_tag_data(std::vector<std::byte> &vertices, std::vector<std::byte> &indices, std::vector<std::byte> &tag_data);

        /**
         * Fix the encounters in the scenario tag
         */
        void fix_scenario_tag_encounters();

        /**
         * Fix the command lists in the scenario tag
         */
        void fix_scenario_tag_command_lists();

        /**
         * Check if the point is in the bsp
         * @param bsp   bsp index to check
         * @param point point to check
         * @return      true if point is inside the bsp
         */
        bool point_in_bsp(std::uint32_t bsp, const HEK::Point3D<HEK::LittleEndian> &point);

        /**
         * Get the leaf index for the point in BSP
         * @param bsp   bsp index to check
         * @param point point to check
         * @return      leaf index or null if not in BSP
         */
        HEK::FlaggedInt<std::uint32_t> leaf_for_point_in_bsp(std::uint32_t bsp, const HEK::Point3D<HEK::LittleEndian> &point);

        bool intersect_in_bsp(const HEK::Point3D<HEK::LittleEndian> &point_a, const HEK::Point3D<HEK::LittleEndian> &point_b, std::uint32_t bsp, HEK::Point3D<HEK::LittleEndian> &intersection_point, std::uint32_t &surface_index, std::uint32_t &leaf_index);

        /**
         * Get the tag index of the BSP
         * @param  bsp bsp index
         * @return     index of the BSP
         */
        std::size_t get_bsp_tag_index(std::uint32_t bsp);

        /**
         * CRC to forge the map to, if required
         */
        std::optional<std::uint32_t> forge_crc;
    };
}
#endif