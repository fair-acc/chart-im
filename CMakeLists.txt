cmake_minimum_required(VERSION 3.20)
project(ImChart CXX)
set(CMAKE_CXX_STANDARD 20)

set(ENV{CPM_SOURCE_CACHE} ${CMAKE_SOURCE_DIR}/.cpm_cache)
include(cmake/CPM.cmake)

CPMAddPackage("https://github.com/fmtlib/fmt.git#8.0.1")
#CPMAddPackage("https://github.com/glfw/glfw.git#3.3.8")

#find_package(GLFW REQUIRED)
find_package(SDL2 REQUIRED)

CPMAddPackage("https://github.com/ocornut/imgui.git#v1.88")
CPMAddPackage("https://github.com/epezent/implot.git#v0.13")

set(BUILD_OPENGL_BACKEND "AUTO" CACHE STRING "Build the OpenGL backend")

if (${EMSCRIPTEN})
    #set(BUILD_OPENGL_BACKEND ON)
    find_package(OpenGL REQUIRED)

    set(OpenGL_FOUND TRUE)
endif()

if (${BUILD_OPENGL_BACKEND} STREQUAL "AUTO")
    find_package(OpenGL)
elseif(${BUILD_OPENGL_BACKEND})
    find_package(OpenGL REQUIRED)
endif()

if (${OpenGL_FOUND})
    CPMAddPackage("https://github.com/Dav1dde/glad.git#v0.1.36")
endif()

add_library(imgui STATIC ${imgui_SOURCE_DIR}/imgui.cpp
                         ${imgui_SOURCE_DIR}/imgui.h
                         ${imgui_SOURCE_DIR}/imgui_demo.cpp
                         ${imgui_SOURCE_DIR}/imgui_draw.cpp
                         ${imgui_SOURCE_DIR}/imgui_tables.cpp
                         ${imgui_SOURCE_DIR}/imgui_widgets.cpp
                         # ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
                         ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(imgui PUBLIC SDL2::SDL2)

if (${OpenGL_FOUND})
    target_sources(imgui PRIVATE ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    target_compile_definitions(imgui PRIVATE -DIMGUI_IMPL_OPENGL_ES2 -DIMGUI_IMPL_OPENGL_LOADER_CUSTOM=1)
    target_compile_definitions(imgui PUBLIC -DIMGUI_USER_CONFIG="${CMAKE_SOURCE_DIR}/src/imconfig.h")
endif()

add_library(implot STATIC ${implot_SOURCE_DIR}/implot.cpp
                          ${implot_SOURCE_DIR}/implot.h
                          ${implot_SOURCE_DIR}/implot_demo.cpp
                          ${implot_SOURCE_DIR}/implot_items.cpp)
target_link_libraries(implot PUBLIC imgui)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})

add_executable(imchart src/main.cpp
                       src/dataset.cpp
                       src/sindataset.cpp
                       src/window.cpp
                       src/timer.cpp
                       src/backends/backend.cpp
                       # src/backends/glfw/glfwbackend.cpp
                       src/backends/sdl/sdlbackend.cpp
                       src/renderers/renderer.cpp)
target_compile_definitions(imchart PRIVATE -DX11_ENABLED)
if (${OpenGL_FOUND})
    target_compile_definitions(imchart PRIVATE -DOPENGL_ENABLED)
    target_sources(imchart PRIVATE src/renderers/opengl/openglrenderer.cpp)
    if (${EMSCRIPTEN}) # NOT doesn't work?!
    else()
        target_link_libraries(imchart OpenGL::GL OpenGL::EGL)
    endif()
endif()

target_include_directories(imchart PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(imchart SDL2::SDL2 fmt::fmt imgui implot)
if (${EMSCRIPTEN})
    set_target_properties(imchart PROPERTIES LINK_FLAGS "-s USE_SDL=2 -s USE_WEBGL2=1 -s FULL_ES3=1 -s ASSERTIONS=1 -sALLOW_MEMORY_GROWTH")
    set_target_properties(imchart PROPERTIES SUFFIX ".html")
endif()
