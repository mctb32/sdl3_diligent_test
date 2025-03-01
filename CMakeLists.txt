cmake_minimum_required (VERSION 3.16)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

project(HelloDiligent CXX)

add_subdirectory(extern/DiligentCore)
add_subdirectory(extern/SDL)

add_executable(SdlDiligentTest WIN32 source/main.cpp source/diligent_sample.cpp)
target_compile_options(SdlDiligentTest PRIVATE -DUNICODE)

target_link_libraries(SdlDiligentTest
PRIVATE
    Diligent-GraphicsEngineD3D11-shared
    Diligent-GraphicsEngineOpenGL-shared
    Diligent-GraphicsEngineD3D12-shared
    Diligent-GraphicsEngineVk-shared
    Diligent-Common
    SDL3::SDL3
)

if(WIN32)
    set_target_properties(SdlDiligentTest PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
endif()

copy_required_dlls(SdlDiligentTest)