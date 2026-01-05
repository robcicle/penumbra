project(penumbra LANGUAGES CXX)
add_library(${PROJECT_NAME} STATIC)

# Output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}/${PROJECT_NAME}/$<CONFIG>/runtime"
    LIBRARY_OUTPUT_DIRECTORY "${BIN_DIR}/${PROJECT_NAME}/$<CONFIG>/library"
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_DIR}/${PROJECT_NAME}/$<CONFIG>/archive"
)

# Dependencies
add_subdirectory(vendor)

set(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Include directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        include
    PRIVATE
        src
)

# Source files
file(GLOB_RECURSE SRC_FILES         src/*.cpp src/ppch.cpp src/penumbra.aps)
file(GLOB_RECURSE PRIVATE_HEADERS   src/*.h)
file(GLOB_RECURSE PUBLIC_HEADERS    include/*.h include/penumbra.rc include/ppch.h)

target_sources(${PROJECT_NAME} PRIVATE
    ${SRC_FILES}
    ${PRIVATE_HEADERS}
    ${PUBLIC_HEADERS}
)

# Precompiled header
target_precompile_headers(${PROJECT_NAME} PRIVATE include/ppch.h)

# Linking
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        TracyClient
        spdlog
        glm
        ImGui
)

# Platform-specific setup
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            d3d11
            dxgi
            d3dcompiler
            dxguid
            Ws2_32.lib
            Winmm.lib
            Version.lib
            Bcrypt.lib
    )
endif()

# Compiler options and defines
if(MSVC)
    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
    target_compile_options(${PROJECT_NAME}
    PUBLIC
        /utf-8
        /Zc:preprocessor
    )
endif()

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        $<$<CONFIG:Debug>:PENUMBRA_DEBUG>
        $<$<CONFIG:Release>:PENUMBRA_RELEASE>
)

# Visual Studio source filters
function(assign_source_group FILES BASE_PATH GROUP_PREFIX)
    foreach(FILE ${FILES})
        get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
        file(RELATIVE_PATH REL_DIR "${BASE_PATH}" "${PARENT_DIR}")
        string(REPLACE "/" "\\" FILTER_GROUP "${GROUP_PREFIX}/${REL_DIR}")
        source_group("${FILTER_GROUP}" FILES "${FILE}")
    endforeach()
endfunction()

assign_source_group("${SRC_FILES}"       "${PROJECT_DIR}/src"     "Source Files")
assign_source_group("${PRIVATE_HEADERS}" "${PROJECT_DIR}/src"     "Header Files")
assign_source_group("${PUBLIC_HEADERS}"  "${PROJECT_DIR}/include" "Header Files")
