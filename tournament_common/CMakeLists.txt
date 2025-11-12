project(tournament_common)

set(CMAKE_CXX_STANDARD 23)

# CAMBIO: Añadimos el nuevo archivo .cpp a la lista de fuentes
set(COMMON_SOURCES
        src/persistence/repository/MatchRepository.cpp
        src/persistence/repository/TournamentRepository.cpp
        src/persistence/repository/GroupRepository.cpp
        src/persistence/repository/domain/IMatchStrategy.cpp)

include_directories(include)


add_library(${PROJECT_NAME} STATIC ${COMMON_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../tournament_services/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../tournament_common/include
    ${HYPODERMIC_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    tournament_logic # La librería que contiene MatchService
    unofficial::activemq-cpp::activemq-cpp
    libpqxx::pqxx
    nlohmann_json::nlohmann_json
)

find_package(libpqxx CONFIG REQUIRED)
target_link_libraries(tournament_common PUBLIC libpqxx::pqxx)

# Agregar headers públicos
target_sources(tournament_common
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
                include/events/Events.hpp
                include/domain/Match.hpp
                include/domain/IMatchStrategy.hpp
                include/persistence/repository/IMatchRepository.hpp
                include/persistence/repository/MatchRepository.hpp
                include/persistence/repository/TournamentRepository.hpp
                include/persistence/repository/GroupRepository.hpp       
)
