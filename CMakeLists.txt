cmake_minimum_required(VERSION 3.0)
project(k380_fn)

add_executable(k380_fn src/k380_fn_switch.cpp)
target_link_libraries(k380_fn -static-libgcc -static-libstdc++)
