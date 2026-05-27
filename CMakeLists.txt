cmake_minimum_required(VERSION 3.10)
project(TelemetryServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")

find_package(PostgreSQL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(STB REQUIRED stb)

add_executable(server
    src/main.cpp
    src/Mercator.cpp
    src/TelemetryParser.cpp
    src/Database.cpp
    src/TileManager.cpp
    src/NetworkServer.cpp
    src/SessionRenderer.cpp
    src/GuiManager.cpp
    src/Heatmap.cpp
)

target_link_libraries(server PRIVATE
    ${PostgreSQL_LIBRARIES}
    ${CURL_LIBRARIES}
    ${STB_LIBRARIES}
    zmq
    SDL2::SDL2
    GLEW::GLEW
    OpenGL::GL
    ${CMAKE_DL_LIBS}
    stdc++fs
)

target_include_directories(server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${PostgreSQL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${STB_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/third_party/imgui
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends
    ${CMAKE_SOURCE_DIR}/third_party/implot
)

set(IMGUI_CORE
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
)

set(IMGUI_BACKENDS
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_sdl2.cpp
    ${CMAKE_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_opengl3.cpp
)

set(IMPLOT_SOURCES
    ${CMAKE_SOURCE_DIR}/third_party/implot/implot.cpp
    ${CMAKE_SOURCE_DIR}/third_party/implot/implot_items.cpp
)

target_sources(server PRIVATE ${IMGUI_CORE} ${IMGUI_BACKENDS} ${IMPLOT_SOURCES})