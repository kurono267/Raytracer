cmake_minimum_required(VERSION 3.13)
project(Raytracer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/Mango)
include_directories(external/Mango/src)

link_directories($ENV{VULKAN_SDK}/lib/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
include_directories(${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -mavx")

set(SCENE_SRC
        src/scene/BVH.cpp
        src/scene/Material.cpp
        src/scene/Model.cpp
        src/scene/Scene.cpp
        src/scene/MeshCPU.cpp
        src/cpu/PathTracer.cpp src/cpu/PathTracer.hpp src/scene/Ray.hpp src/scene/BBox.hpp src/scene/BBox.cpp src/scene/Ray.cpp src/scene/ImagesCache.cpp src/scene/ImagesCache.hpp src/cpu/bsdf/BxDF.hpp src/cpu/bsdf/Diffuse.cpp src/cpu/bsdf/Diffuse.hpp src/cpu/bsdf/BxDF.cpp src/cpu/bsdf/Samples.hpp src/cpu/bsdf/BSDF.cpp src/cpu/bsdf/BSDF.hpp)

add_executable(Raytracer src/app/main.cpp src/app/App.cpp ${SCENE_SRC})
target_link_libraries(Raytracer mango boost_filesystem boost_system)