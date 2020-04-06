# CMakeList.txt: проект CMake для CMakeProject1; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.10)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (CMakeProject1 "booking.h" "test.cpp" "profile.h" "test_runner.h" 
"new_booking_providers.h" "new_trip_manager.h" "old_booking_providers.h" "old_trip_manager.h") 

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
