# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED ${INVADER_BUILD})
    set(INVADER_BUILD true CACHE BOOL "Build invader-build (builds cache files)")
endif()

if(${INVADER_BUILD})
    add_executable(invader-build
        src/build/build.cpp
    )
    target_link_libraries(invader-build invader)

    set(TARGETS_LIST ${TARGETS_LIST} invader-build)
endif()
