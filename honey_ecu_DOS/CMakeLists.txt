cmake_minimum_required(VERSION 3.5)
project(honey_ecu_DOS)

# vsomeip3 라이브러리 찾기
find_package(vsomeip3 REQUIRED)

# DoS 공격을 받는 허니팟 ECU
add_executable(honey_ecu_dos hoeny_ecu_dos.cpp)
target_link_libraries(honey_ecu_dos vsomeip3)

# DoS 공격을 보내는 클라이언트
add_executable(client_dos client_dos.cpp)
target_link_libraries(client_dos vsomeip3)
