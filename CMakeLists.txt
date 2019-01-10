cmake_minimum_required(VERSION 3.13)
project(Raytracer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/Mango)
include_directories(external/Mango/src)

link_directories($ENV{VULKAN_SDK}/lib/)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
include_directories(${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIR} src external/tinygltf external/simd/src)

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
src/bsdf/BxDF.hpp
src/bsdf/Diffuse.cpp
src/bsdf/Diffuse.hpp
src/bsdf/BxDF.cpp
src/bsdf/Samples.hpp
src/bsdf/BSDF.cpp
src/bsdf/BSDF.hpp
src/lights/LightSource.hpp
src/lights/PointLight.cpp
src/lights/PointLight.hpp
src/lights/EnvLight.cpp
src/lights/EnvLight.hpp
src/samplers/Distributions.cpp
src/samplers/Distributions.hpp
src/scene/Common.hpp
src/bsdf/Microfacet.cpp
src/bsdf/Microfacet.hpp
src/cpu/BDPT.cpp
src/cpu/BDPT.hpp
src/cpu/Integrator.hpp
src/cpu/Integrator.cpp src/cpu/PathTracer16.cpp src/cpu/PathTracer16.hpp)

add_executable(Raytracer src/app/main.cpp src/app/App.cpp ${SCENE_SRC})
target_link_libraries(Raytracer mango boost_filesystem boost_system)