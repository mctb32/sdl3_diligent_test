cmake_minimum_required (VERSION 3.6)

project(HelloDiligent CXX)

add_subdirectory(extern/DiligentCore)
add_subdirectory(extern/SDL)

add_executable(SdlDiligentTest WIN32 source/main.cpp)
target_compile_options(SdlDiligentTest PRIVATE -DUNICODE)

target_link_libraries(SdlDiligentTest
PRIVATE
    Diligent-GraphicsEngineD3D11-shared
    Diligent-GraphicsEngineOpenGL-shared
    Diligent-GraphicsEngineD3D12-shared
    Diligent-GraphicsEngineVk-shared
    SDL3::SDL3
)
copy_required_dlls(SdlDiligentTest)