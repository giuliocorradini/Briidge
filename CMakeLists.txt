cmake_minimum_required(VERSION 3.16)
project(Briidge)

set(CMAKE_CXX_STANDARD 14)

add_library(wiiuse SHARED IMPORTED)
set_target_properties(wiiuse PROPERTIES IMPORTED_LOCATION /usr/local/Cellar/wiiuse/0.15.5/lib/libwiiuse.dylib)


add_executable(Briidge main.cpp)
target_include_directories(Briidge PUBLIC
        /usr/local/Cellar/wiiuse/0.15.5/include/)
target_link_libraries(Briidge wiiuse "-framework CoreFoundation" "-framework CoreGraphics")