﻿# CMakeList.txt: проект CMake для CMakeProject1; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.10)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (CMakeProject1 "main.cpp" "test_runner.h" "manager.h" "utils.h" "requests.h" "json.cpp" "json.h" "graph.h" "router.h")

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
