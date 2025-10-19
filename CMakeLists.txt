cmake_minimum_required(VERSION 3.22)

project(char-com)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3 -O0 -Wall -Wextra")


set(ascio-obj
${CMAKE_}
    ./asio/asio/src/asio_ssl.cpp    
    ./asio/asio/src/asio.cpp    
)

set(char-com-obj
    "main.cc"
    "rtt_client.hpp"
    "app.hpp"
    # "rtt_client.c"
    "csv.hpp"
    # "ui.c"
)


add_subdirectory(raylib)
add_executable(char-com 
    "${char-com-obj}"
)

# Asio (standalone, no SSL)
target_include_directories(char-com PRIVATE ${CMAKE_SOURCE_DIR}/asio/asio/include)
target_compile_definitions(char-com PRIVATE
#    WIN32_LEAN_AND_MEAN
#     NOMINMAX
#     NOGDI        # hides GDI's Rectangle() that conflicts with Raylib's Rectangle type
#     NOUSER       # hides WinUser’s CloseWindow/ShowCursor that clash with Raylib APIs
    ASIO_STANDALONE
    ASIO_DISABLE_SSL        # <- force SSL off
    _WIN32_WINNT=0x0A00     # target Windows 10 APIs
)

# target_include_directories(char-com PUBLIC ./asio/asio/include)

target_link_libraries(char-com PUBLIC raylib)

if(WIN32)
    target_link_libraries(char-com PRIVATE ws2_32)
endif()