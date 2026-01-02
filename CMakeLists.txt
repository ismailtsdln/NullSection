cmake_minimum_required(VERSION 3.20)
project(NullSection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check for MSVC since this is a Windows-specific project
if(MSVC)
    add_compile_options(/W4 /WX /permissive- /std:c++20)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    # Clang-cl or cross-compilation settings would go here
    add_compile_options(-Wall -Wextra -Werror)
endif()

# NullSection Core library
    src/core/nt_api.cpp
    src/core/syscalls.cpp
    src/image/pe_parser.cpp
    src/image/image_mapper.cpp
    src/image/section_builder.cpp
    src/process/process_factory.cpp
    src/process/peb_builder.cpp
    src/process/thread_launcher.cpp
    src/evasion/telemetry.cpp
    src/utils/logging.cpp
    src/utils/ntstatus.cpp
    src/core/object_manager.cpp
)

target_include_directories(nullsection_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# CLI Executable
add_executable(nullsection src/cli/main.cpp)
target_link_libraries(nullsection PRIVATE nullsection_core)

# Add NTDLL for resolution
if(WIN32)
    target_link_libraries(nullsection_core PUBLIC ntdll.lib)
endif()
