cmake_minimum_required(VERSION 3.16.3)

project(glad)

add_library(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include/)
target_sources(${PROJECT_NAME} PRIVATE src/glad.c)
