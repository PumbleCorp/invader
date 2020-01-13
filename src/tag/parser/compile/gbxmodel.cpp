// SPDX-License-Identifier: GPL-3.0-only

#include <invader/build/build_workload.hpp>
#include <invader/tag/parser/parser.hpp>

namespace Invader::Parser {
    void GBXModel::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        bool model_part_warned = false;
        for(auto &g : this->geometries) {
            for(auto &p : g.parts) {
                std::size_t compressed_vertex_count = p.compressed_vertices.size();
                std::size_t uncompressed_vertex_count = p.uncompressed_vertices.size();
                if(uncompressed_vertex_count != compressed_vertex_count && compressed_vertex_count != 0) {
                    REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Compressed vertex count (%zu) is not equal to uncompressed (%zu)", compressed_vertex_count, uncompressed_vertex_count)
                    eprintf_warn("To fix this, rebuild the model tag");
                    model_part_warned = true;
                    break;
                }
            }
            if(model_part_warned) {
                break;
            }
        }

        // Swap this stuff
        float super_low = this->super_low_detail_cutoff;
        float low = this->low_detail_cutoff;
        float high = this->high_detail_cutoff;
        float super_high = this->super_high_detail_cutoff;

        this->super_low_detail_cutoff = super_high;
        this->low_detail_cutoff = high;
        this->high_detail_cutoff = low;
        this->super_high_detail_cutoff = super_low;

        if(this->markers.size() > 0) {
            workload.report_error(BuildWorkload::ErrorType::ERROR_TYPE_WARNING, "Markers array is populated, but this array should be empty", tag_index);
            eprintf_warn("To fix this, rebuild the model tag");
        }

        // Put all of the markers in the marker array
        for(auto &r : this->regions) {
            for(auto &p : r.permutations) {
                for(auto &m : p.markers) {
                    GBXModelMarkerInstance instance;
                    instance.node_index = m.node_index;
                    instance.permutation_index = &p - r.permutations.data();
                    instance.region_index = &r - this->regions.data();
                    instance.rotation = m.rotation;
                    instance.translation = m.translation;

                    // Add it!
                    bool found = false;
                    for(auto &ma : this->markers) {
                        if(m.name == ma.name) {
                            ma.instances.push_back(instance);
                            found = true;
                            break;
                        }
                    }

                    // Add the marker and then add it!
                    if(!found) {
                        GBXModelMarker ma = {};

                        ma.name = m.name;
                        ma.instances.push_back(instance);

                        for(auto &marker_group : this->markers) {
                            if(std::strcmp(marker_group.name.string, m.name.string) > 0) {
                                this->markers.insert(this->markers.begin() + (&marker_group - this->markers.data()), ma);
                                found = true;
                                break;
                            }
                        }

                        if(!found) {
                            this->markers.push_back(ma);
                        }
                    }
                }
            }
        }

        // Set node stuff
        std::size_t node_count = this->nodes.size();
        std::vector<bool> node_done(node_count);
        auto *nodes = this->nodes.data();

        auto write_node_data = [&node_done, &nodes, &node_count, &workload, &tag_index](HEK::Index node_index, const auto &base_rotation, const auto &base_translation, const auto &recursion) {
            if(node_index == NULL_INDEX) {
                return;
            }
            if(node_done[node_index]) {
                return;
            }
            node_done[node_index] = true;

            auto &node = nodes[node_index];
            node.scale = 1.0f;

            auto node_rotation = quaternion_to_matrix(node.default_rotation);
            auto total_rotation = multiply_matrix(base_rotation, node_rotation);
            node.rotation = total_rotation;

            auto node_translation = multiply_vector(node.default_translation, -1.0);
            auto total_translation = rotate_vector(add_vector(node_translation, base_translation), node_rotation);
            node.translation = total_translation;

            bool fatal = false;
            if(node.next_sibling_node_index != NULL_INDEX && node.next_sibling_node_index >= node_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Next sibling node index of node %u is invalid (%u >= %zu)", node_index, node.next_sibling_node_index, node_count);
                fatal = true;
            }
            if(node.first_child_node_index != NULL_INDEX && node.first_child_node_index >= node_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "First child node index of node %u is invalid (%u >= %zu)", node_index, node.first_child_node_index, node_count);
                fatal = true;
            }
            if(fatal) {
                throw InvalidTagDataException();
            }

            recursion(node.next_sibling_node_index, base_rotation, base_translation, recursion);
            recursion(node.first_child_node_index, total_rotation, total_translation, recursion);
        };

        HEK::Matrix<HEK::LittleEndian> identity = {};
        for(int i = 0; i < 3; i++) {
            identity.matrix[i][i] = 1.0f;
        }
        HEK::Vector3D<HEK::LittleEndian> no_translation = {};
        write_node_data(0, identity, no_translation, write_node_data);

        // exodux compatibility - recalibrate the bitmask using a high pass filter on the exodux compatibility bit
        bool exodux_handler = false;
        bool exodux_parser = false;

        for(auto &g : this->geometries) {
            for(auto &p : g.parts) {
                // exodux compatibility bit; AND zoner flag with the value from the tag data and XOR with the auxiliary rainbow bitmask
                std::uint32_t zoner = p.flags.zoner;
                std::uint32_t exodux_value = (p.bullshit & zoner) ^ 0x7F7F7F7F;
                if(exodux_handler) {
                    // Since the exodux handler is active, we don't need to use the binary rainbow table for this value.
                    exodux_value ^= 0x3C170A5E;
                }
                else {
                    // Remodulate the upper 16 bits of the control magic since the exodux handler is not active
                    exodux_value <<= 16;

                    // Depending on if the parser is active, activate the precalculated bitmasks from the binary rainbow table
                    exodux_value ^= exodux_parser ? 0x2D1E6921 : 0x291E7021;
                    exodux_parser = !exodux_parser;
                }

                // Invert the last bit if using zoner mode
                if(zoner) {
                    exodux_value ^= 1;
                }

                exodux_handler = !exodux_handler;

                // Do an endian swap of the exodux rainbow table checksum hash
                p.bullshit = (exodux_value & 0xFF000000) >> 24 |
                             (exodux_value & 0xFF0000) >> 8 |
                             (exodux_value & 0xFF00) << 8 |
                             (exodux_value & 0xFF) << 24;
            }
        }
    }

    void GBXModelGeometryPart::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        std::vector<HEK::Index> triangle_indices;

        // Add 1 to this
        workload.part_count++;

        // Add it all
        triangle_indices.reserve(this->triangles.size() * 3);
        for(auto &t : this->triangles) {
            triangle_indices.push_back(t.vertex0_index);
            triangle_indices.push_back(t.vertex1_index);
            triangle_indices.push_back(t.vertex2_index);
        }

        // Remove excess NULL_INDEX values
        std::size_t triangle_indices_size;
        while(true) {
            // Make sure we have enough indices for a triangle
            triangle_indices_size = triangle_indices.size();
            if(triangle_indices_size < 3) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Triangle index count is invalid (%zu < 3)", triangle_indices_size);
                throw InvalidTagDataException();
            }

            // Here we go again
            auto &index = triangle_indices[triangle_indices_size - 1];
            if(index == NULL_INDEX) {
                triangle_indices.erase(triangle_indices.begin() + (&index - triangle_indices.data()));
            }
            else {
                break;
            }
        }

        // Subtract two (since each index is technically an individual triangle, minus the last two indices since you need three indices to make a triangle)
        this->triangle_count = triangle_indices_size - 2;

        // Make sure every triangle is valid
        std::size_t uncompressed_vertices_count = this->uncompressed_vertices.size();
        for(auto &t : triangle_indices) {
            if(t >= uncompressed_vertices_count) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Index #%zu in triangle indices is invalid (%zu >= %zu)", &t - triangle_indices.data(), static_cast<std::size_t>(t), uncompressed_vertices_count);
                throw InvalidTagDataException();
            }
        }

        // See if we can find a copy of this
        std::size_t this_indices_count = triangle_indices_size;
        std::size_t indices_count = workload.model_indices.size();
        bool found = false;

        if(indices_count >= this_indices_count) {
            auto &first = triangle_indices[0];
            auto &last = triangle_indices[triangle_indices_size - 1];
            std::size_t check_size = this_indices_count - 1;

            for(std::size_t i = 0; i <= indices_count - this_indices_count; i++) {
                auto *model_data = workload.model_indices.data() + i;

                // Check the last index, first, since it's most likely to be different
                if(model_data[triangle_indices_size - 1] != last) {
                    continue;
                }

                // If triangles match, set the triangle offset to this instead
                if(std::memcmp(&first, model_data, sizeof(workload.model_indices[0]) * check_size) == 0) {
                    found = true;
                    this->triangle_offset = i * sizeof(workload.model_indices[0]);
                    break;
                }
            }
        }

        if(!found) {
            this->triangle_offset = indices_count * sizeof(workload.model_indices[0]);
            workload.model_indices.insert(workload.model_indices.end(), triangle_indices.begin(), triangle_indices.end());
        }
        this->triangle_offset_2 = this->triangle_offset;

        // Add the vertices, next
        this->vertex_count = this->uncompressed_vertices.size();
        std::vector<GBXModelVertexUncompressed::struct_little> vertices_of_fun;
        vertices_of_fun.reserve(this->vertex_count);
        for(auto &v : this->uncompressed_vertices) {
            auto &mv = vertices_of_fun.emplace_back();
            mv.binormal = v.binormal;
            mv.node0_index = v.node0_index;
            if(v.node1_index == NULL_INDEX) {
                mv.node1_index = 0;
            }
            else {
                mv.node1_index = v.node1_index;
            }
            mv.node0_weight = v.node0_weight;
            mv.node1_weight = v.node1_weight;
            mv.normal = v.normal;
            mv.position = v.position;
            mv.tangent = v.tangent;
            mv.texture_coords = v.texture_coords;
        }

        // Let's see if we can also dedupe this
        std::size_t this_vertices_count = vertices_of_fun.size();
        std::size_t vertices_count = workload.model_vertices.size();
        found = false;

        if(vertices_count >= this_vertices_count) {
            for(std::size_t i = 0; i <= vertices_count - this_vertices_count; i++) {
                // If vertices match, set the vertices offset to this instead
                if(std::memcmp(workload.model_vertices.data() + i, vertices_of_fun.data(), sizeof(workload.model_vertices[0]) * this_vertices_count) == 0) {
                    found = true;
                    this->vertex_offset = i * sizeof(workload.model_vertices[0]);
                    break;
                }
            }
        }

        if(!found) {
            this->vertex_offset = vertices_count * sizeof(workload.model_vertices[0]);
            workload.model_vertices.insert(workload.model_vertices.end(), vertices_of_fun.begin(), vertices_of_fun.end());
        }

        // Don't forget to set these memes
        this->do_not_crash_the_game = 1;
        this->do_not_screw_up_the_model = 4;
        this->prev_filthy_part_index = -1;
        this->next_filthy_part_index = -1;
    }

    void GBXModelRegionPermutation::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        const char *last_hyphen = nullptr;
        for(const char &c : this->name.string) {
            if(c == '-') {
                last_hyphen = &c + 1;
            }
        }
        this->permutation_number = 0;
        if(last_hyphen && *last_hyphen) {
            unsigned long permutation_number = ~0;
            try {
                permutation_number = std::stoul(last_hyphen, nullptr, 10);
                if(permutation_number < static_cast<std::size_t>(NULL_INDEX)) {
                    this->permutation_number = static_cast<HEK::Index>(permutation_number);
                }
            }
            catch(std::out_of_range &) {
                permutation_number = ~0;
            }
            catch(std::exception &) {
                return;
            }
            if(permutation_number >= NULL_INDEX) {
                REPORT_ERROR_PRINTF(workload, ERROR_TYPE_WARNING, tag_index, "Permutation %s has an index that is out of range (%lu >= %zu)", this->name.string, permutation_number, static_cast<std::size_t>(NULL_INDEX));
            }
        }
    }
}