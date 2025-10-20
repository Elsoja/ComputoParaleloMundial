find_package(GTest REQUIRED)

# Define únicamente los archivos de prueba que existen
add_executable(
    tournament_tests_runner
    delegate/TeamDelegateTest.cpp
    controller/TournamentControllerTest.cpp
    delegate/TournamentDelegateTest.cpp
    controller/TeamControllerTest.cpp
    controller/GroupControllerTest.cpp
    delegate/GroupDelegateTest.cpp
)

set_target_properties(tournament_tests_runner PROPERTIES CXX_STANDARD 23)

# Enlaza las librerías necesarias para las pruebas
target_link_libraries(
    tournament_tests_runner
    PRIVATE
    tournament_logic  # La librería con el código que queremos probar
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)

# Habilita el descubrimiento automático de pruebas
include(GoogleTest)
gtest_discover_tests(tournament_tests_runner)