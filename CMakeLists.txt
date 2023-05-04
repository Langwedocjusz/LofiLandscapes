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

#Add preprocessor definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")

#Specify source files
file(GLOB_RECURSE headers src/*.h)
file(GLOB_RECURSE sources src/*.cpp)
#Glfw implementation files for imgui
file(GLOB imgui_impl vendor/imgui/backends/imgui_impl_opengl3.cpp vendor/imgui/backends/imgui_impl_glfw.cpp)

target_sources(${PROJECT_NAME} PRIVATE ${headers} ${sources} ${imgui_impl})

#Specify include directories
target_include_directories(${PROJECT_NAME} PUBLIC src)

#Directory structure for IDEs like Visual Studio
source_group(src REGULAR_EXPRESSION "src/*")
source_group(src/subrenderers REGULAR_EXPRESSION "src/subrenderers/*")
source_group(imgui_impl FILES ${imgui_impl})

#Set current project as Visual Studio start project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
#Set Visual Studio Debugger working directory
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#Add glfw
add_subdirectory(vendor/glfw)
target_include_directories(${PROJECT_NAME} PUBLIC vendor/glfw/include)
target_link_directories(${PROJECT_NAME} PRIVATE vendor/glfw/src)
target_link_libraries(${PROJECT_NAME} glfw)

#Add glad
add_subdirectory(vendor/glad)
target_include_directories(${PROJECT_NAME} PUBLIC vendor/glad/include)
target_link_directories(${PROJECT_NAME} PRIVATE vendor/glad/src)
target_link_libraries(${PROJECT_NAME} glad)

#Add imgui
add_subdirectory(vendor/imgui)
target_include_directories(${PROJECT_NAME} PUBLIC vendor/imgui)
target_link_directories(${PROJECT_NAME} PRIVATE vendor/imgui)
target_link_libraries(${PROJECT_NAME} imgui)

#Add glm
target_include_directories(${PROJECT_NAME} PUBLIC vendor/glm)

#Add shader-include
target_include_directories(${PROJECT_NAME} PUBLIC vendor/GLSL-Shader-Includes)

#Add nlohmann json
target_include_directories(${PROJECT_NAME} PUBLIC vendor/json/single_include)
