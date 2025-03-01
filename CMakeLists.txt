cmake_minimum_required (VERSION 3.16)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

project(HelloDiligent CXX)

add_subdirectory(extern/DiligentCore)
add_subdirectory(extern/SDL)

add_executable(SdlDiligentTest WIN32 source/main.cpp source/diligent_sample.cpp source/diligent_sample.h)
target_compile_options(SdlDiligentTest PRIVATE -DUNICODE)

if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(SdlDiligentTest PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(SdlDiligentTest
PRIVATE
    Diligent-GraphicsEngineD3D11-shared
    Diligent-GraphicsEngineOpenGL-shared
    Diligent-GraphicsEngineD3D12-shared
    Diligent-GraphicsEngineVk-shared
    Diligent-Common
    SDL2::SDL2
)

if(WIN32)
    set_target_properties(SdlDiligentTest PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

copy_required_dlls(SdlDiligentTest)