# 1. Activar y hacer visibles los paquetes de GTest que vcpkg instaló
find_package(GTest REQUIRED)

# 2. Definir el ejecutable de pruebas con TODOS sus archivos fuente.
add_executable(
    tournament_tests_runner
    controller/TeamControllerTest.cpp
    delegate/TeamDelegateTest.cpp
)

# 3. Enlazar las librerías necesarias.
target_link_libraries(
    tournament_tests_runner
    PRIVATE
    tournament_logic  # La librería con nuestro código a probar
    GTest::gtest
    GTest::gmock
    GTest::gtest_main
)

# 4. Habilitar el descubrimiento de pruebas con CTest.
include(GoogleTest)
gtest_discover_tests(tournament_tests_runner)