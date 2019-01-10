//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include "BBox.hpp"
#include "simd/Float16.hpp"

struct Ray {
    Ray() = default;
    Ray(const glm::vec3& _org, const glm::vec3& _dir) : org(_org),dir(_dir){
        invdir = 1.0f/dir;
        for(int i = 0;i<3;++i){
            sign[i] = invdir[i] < 0;
            if(dir[i] == 0.0f)invdir[i] = 0.0f;
        }
    }

    glm::vec3 org;
    glm::vec3 dir;
    glm::vec3 invdir;
    glm::ivec3 sign;
};

struct RayHit {
    RayHit();

    bool status;
    float dist;
    int id0;
    int id1;

    glm::vec3 bc;
};

RayHit intersectBBox(const Ray& ray,const BBox& box);
RayHit intersectTriangle(const Ray& ray,const glm::vec3& v0,const glm::vec3& v1,const glm::vec3& v2);

// Ray16

using namespace simd;

typedef glm::vec<4, Float16, glm::defaultp> vec4f16;
typedef glm::vec<3, Float16, glm::defaultp> vec3f16;
typedef glm::vec<2, Float16, glm::defaultp> vec2f16;

typedef glm::vec<3, Bool16, glm::defaultp> bvec3f16;

struct Ray16 {
    Ray16() = default;
    Ray16(const vec3f16& _org, const vec3f16& _dir) : org(_org),dir(_dir){
        invdir = simd::Float16(1.0f)/dir;
        for(int i = 0;i<3;++i){
            sign[i] = invdir[i] < 0.f;
            If(dir[i] == 0.0f,[&](){
                invdir[i] = 0.0f;
            });
        }
    }

    vec3f16 org;
    vec3f16 dir;
    vec3f16 invdir;
    bvec3f16 sign;
};

struct RayHit16 {
    RayHit16();

    Float16 status;
    Float16 dist;
    Float16 id0;
    Float16 id1;

    vec3f16 bc;
};

inline vec3f16 max(const vec3f16& a, const vec3f16& b){
    return vec3f16(max(a.x,b.x),max(a.y,b.y),max(a.z,b.z));
}

inline vec3f16 min(const vec3f16& a, const vec3f16& b){
    return vec3f16(min(a.x,b.x),min(a.y,b.y),min(a.z,b.z));
}

inline vec3f16 cross(const vec3f16& a, const vec3f16& b){
    vec3f16 result;
    result[0] = result[1] * result[2] - result[2] * result[1];
    result[1] = result[0] * result[2] - result[2] * result[0];
    result[2] = result[0] * result[1] - result[1] * result[0];
    return result;
}

inline Float16 dot(const vec3f16& a, const vec3f16& b){
    Float16 result = 0.f;
    for (int i = 0; i < 3; i++)
        result = result + a[i] * b[i];
    return result;
}

RayHit16 intersectBBox(const Ray16& ray,const BBox& box);
RayHit16 intersectTriangle(const Ray16& ray,const glm::vec3& v0,const glm::vec3& v1,const glm::vec3& v2);
