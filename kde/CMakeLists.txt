cmake_minimum_required(VERSION 3.5)

project(kde-alt-shift-kbd-layout-switcher C)

find_package(PkgConfig)
pkg_search_module(X11 REQUIRED x11)

set(SOURCES main.c)
set(HEADERS )

link_directories(${X11_LIBRARY_DIRS})
include_directories(${X11_INCLUDE_DIR})

add_executable(kde-alt-shift-kbd-layout-switcher ${SOURCES} ${HEADERS})
target_link_libraries(kde-alt-shift-kbd-layout-switcher ${X11_LIBRARIES} -lXi -pthread)

install(TARGETS kde-alt-shift-kbd-layout-switcher RUNTIME DESTINATION bin)
install(FILES kde-alt-shift-kbd-layout-switcher.desktop DESTINATION /etc/xdg/autostart)
