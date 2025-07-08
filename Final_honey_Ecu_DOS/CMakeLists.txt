cmake_minimum_required(VERSION 3.5)
project(honey_dos_simulation)

# vsomeip3 라이브러리 찾기
find_package(vsomeip3 REQUIRED)

# 허니팟 ECU 서버
add_executable(honey_ecu honey_ecu.cpp)
target_link_libraries(honey_ecu vsomeip3)

# DoS 공격 클라이언트
add_executable(client_dos client_dos.cpp)
target_link_libraries(client_dos vsomeip3)
