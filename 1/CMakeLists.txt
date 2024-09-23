cmake_minimum_required(VERSION 2.6)
project(networks_1)

# include(cmake/App.cmake)

add_compile_options("/std:c++17")

# add_app(main.cpp)
add_executable(main main.cpp app.cpp)

target_include_directories(main PRIVATE "./" zlib "ultralight/include")
add_subdirectory(zlib)

target_link_directories(main PRIVATE "ultralight/lib")
target_link_libraries(main PRIVATE Ws2_32 Mswsock AdvApi32 zlibstatic)
target_link_libraries(main PRIVATE AppCore Ultralight UltralightCore WebCore)

set(CMAKE_INSTALL_PREFIX ".")
install(TARGETS main DESTINATION .)
install(FILES ultralight/bin/AppCore.dll ultralight/bin/Ultralight.dll ultralight/bin/UltralightCore.dll ultralight/bin/WebCore.dll DESTINATION .)