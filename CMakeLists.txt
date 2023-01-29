cmake_minimum_required(VERSION 3.12)
project(snakerl DESCRIPTION "Snake")

find_package(SDL2)
set(CMAKE_EXPORT_COMPILE_COMMANDS yes)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED yes)
set(CMAKE_C_EXTENSIONS no)

# Examples
add_executable(snakerl main.c game.c ui.c const.c)

include_directories(${SDL2_INCLUDE_DIRS})

target_link_libraries(snakerl ${SDL2_LIBRARIES})
target_link_libraries(snakerl m)
