# CMakeList.txt: проект CMake для CMakeProject1; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.10)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (CMakeProject1 "collide.cpp" "profile.h" "test_runner.h" "geo2d.cpp" "geo2d.h" "game_object.h" )

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
