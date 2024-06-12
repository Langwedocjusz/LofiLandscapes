cmake_minimum_required(VERSION 3.16.3)

#Set the build output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#Print selected build type / available build types in case of Visual Studio
if (CMAKE_GENERATOR MATCHES "Visual Studio")
	message(STATUS "Build types: ${CMAKE_CONFIGURATION_TYPES}")
else()
	message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

#Set the project name (and version)
project(LofiLandscapes)

#Specify CPP standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#Build current project as an executable
add_executable(${PROJECT_NAME})

#Specify source files
file(GLOB_RECURSE headers src/*.h)
file(GLOB_RECURSE sources src/*.cpp)
#Glfw implementation files for imgui
file(GLOB imgui_impl vendor/imgui/backends/imgui_impl_opengl3.cpp vendor/imgui/backends/imgui_impl_glfw.cpp)

target_sources(${PROJECT_NAME} PRIVATE ${headers} ${sources} ${imgui_impl})

#Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_include_directories(${PROJECT_NAME} PUBLIC src/gui)

#Add preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

#Enable more warnings
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

#Add dependencies
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)
add_subdirectory(vendor/imgui)
add_subdirectory(vendor/json)

target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} json)

#Glm currently added as include directory, since their cmake doesn't suppress warnings
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE vendor/glm)

#Directory structure for IDEs like Visual Studio
source_group(src REGULAR_EXPRESSION "src/*")
source_group(src/subrenderers REGULAR_EXPRESSION "src/subrenderers/*")
source_group(src/gui REGULAR_EXPRESSION "src/gui/*")
source_group(imgui_impl FILES ${imgui_impl})

#Set current project as Visual Studio start project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
#Set Visual Studio Debugger working directory
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
