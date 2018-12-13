cmake_minimum_required(VERSION 3.13)
project(Raytracer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/Mango)
include_directories(external/Mango/src)

link_directories($ENV{VULKAN_SDK}/lib/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
include_directories(${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIR})

add_executable(Raytracer src/app/main.cpp src/app/App.cpp)
target_link_libraries(Raytracer mango)