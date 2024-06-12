cmake_minimum_required(VERSION 3.16.3)

project(json)

add_library(json INTERFACE)
target_include_directories(json SYSTEM INTERFACE single_include)