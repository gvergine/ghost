cmake_minimum_required (VERSION 2.8)

project(ghost)

SET(SRC ghost.c)

add_executable(ghost ${SRC})
target_link_libraries(ghost th)