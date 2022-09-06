cmake_minimum_required(VERSION 3.20)
project(ImChart CXX)
set(CMAKE_CXX_STANDARD 20)

set(ENV{CPM_SOURCE_CACHE} ${CMAKE_SOURCE_DIR}/.cpm_cache)
include(cmake/CPM.cmake)

CPMAddPackage("https://github.com/fmtlib/fmt.git#8.0.1")
CPMAddPackage("https://github.com/glfw/glfw.git#3.3.8")
CPMAddPackage("https://github.com/ocornut/imgui.git#v1.88")
CPMAddPackage("https://github.com/epezent/implot.git#v0.13")

set(BUILD_OPENGL_BACKEND "AUTO" CACHE STRING "Build the OpenGL backend")

if (${BUILD_OPENGL_BACKEND} STREQUAL "AUTO")
    find_package(OpenGL)
elseif(${BUILD_OPENGL_BACKEND})
    find_package(OpenGL REQUIRED)
endif()

if (${OpenGL_FOUND})
    CPMAddPackage("https://github.com/Dav1dde/glad.git#v0.1.36")
endif()

add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp
                         ${imgui_SOURCE_DIR}/imgui_demo.cpp
                         ${imgui_SOURCE_DIR}/imgui_draw.cpp
                         ${imgui_SOURCE_DIR}/imgui_tables.cpp
                         ${imgui_SOURCE_DIR}/imgui_widgets.cpp
                         ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

if (${OpenGL_FOUND})
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    target_compile_definitions(imgui PRIVATE -DIMGUI_IMPL_OPENGL_ES3 -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM=1)
endif()

add_executable(imchart src/main.cpp
                       src/window.cpp
                       src/backends/backend.cpp
                       src/backends/glfw/glfwbackend.cpp
                       src/renderers/renderer.cpp)
target_compile_definitions(imchart PRIVATE -DX11_ENABLED)
if (${OpenGL_FOUND})
    target_compile_definitions(imchart PRIVATE -DOPENGL_ENABLED)
    target_sources(imchart PRIVATE src/renderers/opengl/openglrenderer.cpp)
    target_link_libraries(imchart OpenGL::GL OpenGL::EGL)
endif()

target_include_directories(imchart PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(imchart glfw fmt::fmt imgui)
