# SPDX-License-Identifier: GPL-3.0-only

from copy import deepcopy
from compile import make_cache_format_data
from generate_hek_tag_data import make_cpp_save_hek_data
from read_cache_file_data import make_parse_cache_file_data
from read_hek_data import make_parse_hek_tag_data
from read_hek_file import make_parse_hek_tag_file
from cache_deformat_data import make_cache_deformat
from refactor_reference import make_refactor_reference
from parser_struct import make_parser_struct
from check_broken_enums import make_check_broken_enums
from check_invalid_references import make_check_invalid_references
from check_invalid_ranges import make_check_invalid_ranges
from check_invalid_indices import make_check_invalid_indices
from check_normalize import make_normalize
from compare import make_compare

def make_parser(all_enums, all_bitfields, all_structs_arranged, all_structs, extract_hidden, hpp, cpp_save_hek_data, cpp_read_cache_file_data, cpp_read_hek_data, cpp_cache_format_data, cpp_cache_deformat_data, cpp_refactor_reference, cpp_struct_value, cpp_check_broken_enums, cpp_check_invalid_references, cpp_check_invalid_ranges, cpp_check_invalid_indices, cpp_compare, cpp_normalize, cpp_read_hek_file):
    def write_for_all_cpps(what):
        cpp_save_hek_data.write(what)
        cpp_read_cache_file_data.write(what)
        cpp_read_hek_data.write(what)
        cpp_cache_format_data.write(what)
        cpp_cache_deformat_data.write(what)
        cpp_struct_value.write(what)
        cpp_check_broken_enums.write(what)
        cpp_check_invalid_references.write(what)
        cpp_check_invalid_ranges.write(what)
        cpp_refactor_reference.write(what)
        cpp_check_invalid_indices.write(what)
        cpp_compare.write(what)
        cpp_normalize.write(what)
        cpp_read_hek_file.write(what)

    hpp.write("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    write_for_all_cpps("// SPDX-License-Identifier: GPL-3.0-only\n\n// This file was auto-generated.\n// If you want to edit this, edit the .json definitions and rerun the generator script, instead.\n\n")
    header_name = "INVADER__TAG__PARSER__PARSER_HPP"
    hpp.write("#ifndef {}\n".format(header_name))
    hpp.write("#define {}\n\n".format(header_name))
    hpp.write("#include <string>\n")
    hpp.write("#include <optional>\n")
    hpp.write("#include \"../../map/map.hpp\"\n")
    hpp.write("#include \"parser_struct.hpp\"\n\n")
    hpp.write("namespace Invader {\n")
    hpp.write("    class BuildWorkload;\n")
    hpp.write("}\n")
    hpp.write("namespace Invader::Parser {\n")

    write_for_all_cpps("#include <invader/tag/parser/parser.hpp>\n")
    write_for_all_cpps("#include <invader/map/map.hpp>\n")
    write_for_all_cpps("#include <invader/map/tag.hpp>\n")
    write_for_all_cpps("#include <invader/tag/hek/header.hpp>\n")
    write_for_all_cpps("#include <invader/printf.hpp>\n")
    cpp_cache_format_data.write("#include <invader/build/build_workload.hpp>\n")
    cpp_read_cache_file_data.write("#include <invader/file/file.hpp>\n")
    cpp_read_hek_data.write("#include <invader/file/file.hpp>\n")
    cpp_save_hek_data.write("extern \"C\" std::uint32_t crc32(std::uint32_t crc, const void *buf, std::size_t size) noexcept;\n")
    write_for_all_cpps("namespace Invader::Parser {\n")

    for s in all_structs_arranged:
        struct_name = s["name"]
        post_cache_deformat = "post_cache_deformat" in s and s["post_cache_deformat"]
        post_cache_parse = "post_cache_parse" in s and s["post_cache_parse"]
        pre_compile = "pre_compile" in s and s["pre_compile"]
        post_compile = "post_compile" in s and s["post_compile"]
        postprocess_hek_data = "postprocess_hek_data" in s and s["postprocess_hek_data"]
        normalize = "normalize" in s and s["normalize"]
        read_only = "read_only" in s and s["read_only"]
        private_functions = post_cache_deformat
        title = None if "title" not in s else s["title"]

        hpp.write("    struct {} : public ParserStruct {{\n".format(struct_name))
        hpp.write("        using struct_big = HEK::{}<HEK::BigEndian>;\n".format(struct_name))
        hpp.write("        using struct_little = HEK::{}<HEK::LittleEndian>;\n".format(struct_name))
        all_used_groups = s["groups"] if "groups" in s else []
        all_used_structs = []
        
        # First add all of the values
        def add_structs_from_struct(struct):
            if "inherits" in struct:
                for t in all_structs:
                    if t["name"] == struct["inherits"]:
                        add_structs_from_struct(t)
                        if "groups" in t:
                            for g in t["groups"]:
                                all_used_groups.append(deepcopy(g))
                        break
            for t in struct["fields"]:
                if t["type"] == "pad":
                    continue
                type_to_write = t["type"]
                non_type = False
                if type_to_write.startswith("int") or type_to_write.startswith("uint"):
                    type_to_write = "std::{}_t".format(type_to_write)
                    non_type = True
                elif type_to_write == "float":
                    type_to_write = "float"
                    non_type = True
                elif type_to_write == "TagDependency":
                    type_to_write = "Dependency"
                    non_type = True
                elif type_to_write == "TagReflexive":
                    if t["struct"] == "PredictedResource":
                        continue
                    type_to_write = "std::vector<{}>".format(t["struct"])
                    non_type = True
                elif type_to_write == "TagDataOffset":
                    type_to_write = "std::vector<std::byte>"
                    non_type = True
                else:
                    type_to_write = "HEK::{}".format(type_to_write)
                    
                initializer = " = NULL_INDEX" if type_to_write == "HEK::Index" else ""
                
                if "flagged" in t and t["flagged"]:
                    type_to_write = "HEK::FlaggedInt<{}>".format(type_to_write)
                if "compound" in t and t["compound"] and not non_type:
                    type_to_write = "{}<HEK::NativeEndian>".format(type_to_write)
                if "bounds" in t and t["bounds"]:
                    type_to_write = "HEK::Bounds<{}>".format(type_to_write)
                hpp.write("        {} {}{}{};\n".format(type_to_write, t["member_name"], "" if "count" not in t or t["count"] == 1 else "[{}]".format(t["count"]), initializer))
                all_used_structs.append(deepcopy(t))
                continue
        add_structs_from_struct(s)
        
        # Next, account for enums being excluded on different structs
        for s in all_used_structs:
            for q in all_enums:
                if s["type"] == q["name"] and "exclude" in s:
                    excluded = []
                    for i in s["exclude"]:
                        if i["struct"] is None or struct_name in i["struct"]:
                            intval = None
                            for k in range(len(q["options"])):
                                if q["options"][k] == i["option"]:
                                    intval = k
                                    break
                            if intval is None:
                                raise "option {} not found in enum {}".format(i["option"], q["name"])
                            excluded.append(intval)
                    if len(excluded) > 0:
                        s["__excluded"] = excluded
                    else:
                        s["__excluded"] = None
                    break
        
        # Next, account for bitmasks being excluded on different structs
        for s in all_used_structs:
            for q in all_bitfields:
                if s["type"] == q["name"] and "exclude" in s:
                    excluded = 0
                    for i in s["exclude"]:
                        if i["struct"] is None or struct_name in i["struct"]:
                            intval = None
                            for k in range(len(q["fields"])):
                                if q["fields"][k] == i["field"]:
                                    intval = k
                                    break
                            if intval is None:
                                raise "field {} not found in bitmask {}".format(i["field"], q["name"])
                            excluded = excluded | (1 << intval)
                    if excluded > 0:
                        s["__excluded"] = excluded
                    else:
                        s["__excluded"] = None
        
        # Next, run all this stuff to generate our C++ source files
        make_cache_deformat(post_cache_deformat, all_used_structs, struct_name, hpp, cpp_cache_deformat_data)
        make_cache_format_data(struct_name, s, pre_compile, post_compile, all_used_structs, hpp, cpp_cache_format_data, all_enums, all_structs_arranged)
        make_cpp_save_hek_data(extract_hidden, all_bitfields, all_used_structs, struct_name, hpp, cpp_save_hek_data)
        make_parse_cache_file_data(post_cache_parse, all_bitfields, all_used_structs, struct_name, hpp, cpp_read_cache_file_data)
        make_parse_hek_tag_data(postprocess_hek_data, all_bitfields, struct_name, all_used_structs, hpp, cpp_read_hek_data)
        make_parse_hek_tag_file(struct_name, hpp, cpp_read_hek_file)
        make_refactor_reference(all_used_structs, struct_name, hpp, cpp_refactor_reference)
        make_parser_struct(cpp_struct_value, all_enums, all_bitfields, all_used_structs, all_used_groups, hpp, struct_name, extract_hidden, read_only, title)
        make_check_broken_enums(all_enums, all_used_structs, struct_name, hpp, cpp_check_broken_enums)
        make_check_invalid_references(all_used_structs, struct_name, hpp, cpp_check_invalid_references)
        make_check_invalid_ranges(all_used_structs, struct_name, hpp, cpp_check_invalid_ranges)
        make_check_invalid_indices(all_used_structs, struct_name, hpp, cpp_check_invalid_indices, all_structs_arranged)
        make_normalize(all_used_structs, struct_name, hpp, cpp_normalize, normalize)
        make_compare(all_used_structs, struct_name, all_bitfields, hpp, cpp_compare)

        hpp.write("        ~{}() override = default;\n".format(struct_name))

        if postprocess_hek_data:
            hpp.write("        void postprocess_hek_data();\n")

        if post_cache_deformat:
            hpp.write("        void post_cache_deformat();\n")

        if post_cache_parse:
            hpp.write("        void post_cache_parse(const Invader::Tag &, std::optional<HEK::Pointer>);\n")

        if pre_compile:
            hpp.write("        void pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);\n")

        if post_compile:
            hpp.write("        void post_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t struct_index, std::size_t offset);\n")

        hpp.write("    };\n")
    hpp.write("}\n")
    hpp.write("#endif\n")
    write_for_all_cpps("}\n")
    hpp.close()
