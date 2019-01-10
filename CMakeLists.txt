cmake_minimum_required(VERSION 3.13)
project(Raytracer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/Mango)
include_directories(external/Mango/src)

link_directories($ENV{VULKAN_SDK}/lib/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
include_directories(${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIR} src external/tinygltf)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=skylake-avx512")

set(SCENE_SRC
src/scene/BVH.cpp
src/scene/Material.cpp
src/scene/Model.cpp
src/scene/Scene.cpp
src/scene/MeshCPU.cpp
src/cpu/PathTracer.cpp
src/cpu/PathTracer.hpp
src/scene/Ray.hpp
src/scene/BBox.hpp
src/scene/BBox.cpp
src/scene/Ray.cpp
src/scene/ImagesCache.cpp
src/scene/ImagesCache.hpp
src/scene/bsdf/BxDF.hpp
src/scene/bsdf/Diffuse.cpp
src/scene/bsdf/Diffuse.hpp
src/scene/bsdf/BxDF.cpp
src/scene/bsdf/Samples.hpp
src/scene/bsdf/BSDF.cpp
src/scene/bsdf/BSDF.hpp
src/scene/lights/LightSource.hpp
src/scene/lights/PointLight.cpp
src/scene/lights/PointLight.hpp
src/scene/lights/EnvLight.cpp
src/scene/lights/EnvLight.hpp
src/scene/samplers/Distributions.cpp
src/scene/samplers/Distributions.hpp
src/scene/Common.hpp
src/scene/bsdf/Microfacet.cpp
src/scene/bsdf/Microfacet.hpp src/cpu/BDPT.cpp src/cpu/BDPT.hpp src/cpu/Integrator.hpp src/cpu/Integrator.cpp)

add_executable(Raytracer src/app/main.cpp src/app/App.cpp ${SCENE_SRC})
target_link_libraries(Raytracer mango boost_filesystem boost_system)