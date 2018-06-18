cmake_minimum_required(VERSION 3.5)

project(ctrl-shift-layout-switcher C)

find_package(PkgConfig)
pkg_search_module(X11 REQUIRED x11)

set(SOURCES main.c)
set(HEADERS )

link_directories(${X11_LIBRARY_DIRS})
include_directories(${X11_INCLUDE_DIR})

add_executable(ctrl-shift-layout-switcher ${SOURCES} ${HEADERS})
target_link_libraries(ctrl-shift-layout-switcher ${X11_LIBRARIES} -lXi -pthread)

install(TARGETS ctrl-shift-layout-switcher RUNTIME DESTINATION bin)
install(FILES gnome-alt-shift-kbd-layout-switcher.desktop DESTINATION /etc/xdg/autostart)
