project(tournament_services)

# Se mantiene igual, define el estándar C++
set(CMAKE_CXX_STANDARD 23)

# Se mantiene igual, encuentra las dependencias principales
find_package(Crow CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_path(HYPODERMIC_INCLUDE_DIRS "Hypodermic/ActivatedRegistrationInfo.h")
find_package(nlohmann_json CONFIG REQUIRED)

# ===================================================================
# CAMBIO PRINCIPAL: CREAR UNA LIBRERÍA CON LA LÓGICA DE NEGOCIO
# ===================================================================

# 1. Definir los archivos fuente de la lógica (controladores, delegados)
set(SERVICES_LOGIC_SOURCES
        src/delegate/TeamDelegate.cpp
        src/delegate/TournamentDelegate.cpp
        src/controller/TournamentController.cpp
        src/controller/TeamController.cpp
)

# 2. Crear una librería estática llamada "tournament_logic"
add_library(tournament_logic STATIC ${SERVICES_LOGIC_SOURCES})

# 3. Añadir el directorio 'include' a la librería y a todo lo que la use
target_include_directories(tournament_logic PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# 4. Enlazar dependencias que la librería necesita (como nlohmann_json y tournament_common)
target_link_libraries(tournament_logic PUBLIC
    nlohmann_json::nlohmann_json
    tournament_common
)

# ===================================================================
# CREAR EL EJECUTABLE PRINCIPAL USANDO LA LIBRERÍA
# ===================================================================

# 5. Crear el ejecutable SÓLO con main.cpp
add_executable(${PROJECT_NAME} main.cpp)

# 6. Enlazar el ejecutable a la librería de lógica y otras dependencias
target_link_libraries(${PROJECT_NAME} PRIVATE
        tournament_logic # <-- Aquí usamos la librería que creamos
        Crow::Crow
        asio::asio
        libpqxx::pqxx
        unofficial::activemq-cpp::activemq-cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${HYPODERMIC_INCLUDE_DIRS})

# Habilitar pruebas y procesar el subdirectorio 'tests'
include(CTest)
enable_testing()
add_subdirectory(tests)

# La sección de configure_file se mantiene igual
configure_file(
        ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}/configuration.json
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/configuration.json
        COPYONLY)