cmake_minimum_required(VERSION 3.5)
project(honey_ecu)

find_package(vsomeip3 REQUIRED)

add_executable(honey_ecu honey_ecu.cpp)
target_link_libraries(honey_ecu vsomeip3)

add_executable(client client.cpp)
target_link_libraries(client vsomeip3)
