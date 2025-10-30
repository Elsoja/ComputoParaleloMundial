cmake_minimum_required(VERSION 3.31.6)
project(Tournaments)

cmake_policy(SET CMP0135 NEW)

set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(_HAS_AUTO_PTR_ETC=1)

find_package(Crow CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_path(HYPODERMIC_INCLUDE_DIRS "Hypodermic/ActivatedRegistrationInfo.h")
find_package(nlohmann_json CONFIG REQUIRED)
find_package(activemq-cpp REQUIRED)

add_subdirectory(tournament_common)
add_subdirectory(tournament_services)
add_subdirectory(tournament_consumer)
