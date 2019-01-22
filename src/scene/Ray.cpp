//
// Created by kurono267 on 2018-12-14.
//

#include "Ray.hpp"

using namespace glm;

RayHit::RayHit(){
    status = false;
    dist = std::numeric_limits<float>::infinity();
    id0 = -1;
    id1 = -1;
    bc = glm::vec3(0.0f);
}

RayHit intersectBBox(const Ray& ray,const BBox &box) {
    RayHit hit;

    vec3 lov = ray.invdir*(box.min - ray.org);
    vec3 hiv = ray.invdir*(box.max - ray.org);

    vec3 max_v = max(lov, hiv);
    vec3 min_v = min(lov, hiv);

    float tmin  = max(min_v.x,max(min_v.y,min_v.z));//reduce_max(min(lov, hiv));
    float tmax =  min(max_v.x,min(max_v.y,max_v.z));

    if(tmin <= tmax && (tmax > 0.0f)){
        hit.dist = tmin;
        hit.status = true;
    }
    return hit;
}

RayHit intersectTriangle(const Ray& ray,const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2) {
    RayHit hit;

    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    // Calculate planes normal vector
    vec3 pvec = cross(ray.dir, e2);
    float det = dot(e1, pvec);

    // Ray is parallel to plane
    if (abs(det) < 1e-8) {
        return hit;
    }

    float inv_det = 1 / det;
    vec3 tvec = ray.org - v0;
    float u = dot(tvec, pvec) * inv_det;
    if (u < 0.0f || u > 1.0f) {
        return hit;
    }

    vec3 qvec = cross(tvec, e1);
    float v = dot(ray.dir, qvec) * inv_det;
    if (v < 0.0f || u + v > 1.0f) {
        return hit;
    }
    hit.dist = dot(e2, qvec) * inv_det;
    hit.status = true;
    hit.bc = vec3(1.0f-u-v,u,v);

    return hit;
}

RayHit16::RayHit16() {
    status = 0.f;
    dist = std::numeric_limits<float>::infinity();
    id0 = -1.f;
    id1 = -1.f;
    bc = vec3f16(0.0f);
}

RayHit16 intersectBBox(const Ray16 &ray, const BBox &box) {
    RayHit16 hit;

    vec3f16 boxMin(box.min.x,box.min.y,box.min.z);
    vec3f16 boxMax(box.max.x,box.max.y,box.max.z);

    vec3f16 lov = ray.invdir*(boxMin - ray.org);
    vec3f16 hiv = ray.invdir*(boxMax - ray.org);

    vec3f16 max_v = max(lov, hiv);
    vec3f16 min_v = min(lov, hiv);

    Float16 tmin  = max(min_v.x,max(min_v.y,min_v.z));//reduce_max(min(lov, hiv));
    Float16 tmax =  min(max_v.x,min(max_v.y,max_v.z));

    If(tmin <= tmax & (tmax > 0.0f),[&](){
        hit.dist = tmin;
        hit.status = 1.f;
    });
    return hit;
}

RayHit16 intersectTriangle(const Ray16 &ray, const glm::vec3 &_v0, const glm::vec3 &_v1, const glm::vec3 &_v2) {
    RayHit16 hit;

    // Pack vertices
    vec3f16 v0(_v0.x,_v0.y,_v0.z);
    vec3f16 v1(_v1.x,_v1.y,_v1.z);
    vec3f16 v2(_v2.x,_v2.y,_v2.z);

    vec3f16 e1 = v1 - v0;
    vec3f16 e2 = v2 - v0;
    // Calculate planes normal vector
    vec3f16 pvec = cross(ray.dir, e2);
    Float16 det = dot(e1, pvec);

    // Ray is parallel to plane
    If(abs(det) >= 1e-8,[&]() {
        //Float16 inv_det = 1.f / det;
        vec3f16 tvec = ray.org - v0;
        Float16 u = dot(tvec, pvec) / det;
        If((u >= 0.f) & (u <= 1.f),[&]() {
            vec3f16 qvec = cross(tvec, e1);
            Float16 v = dot(ray.dir, qvec) / det;
            If( (v >= 0.0f) & ((u + v) <= 1.0f), [&]() {
                hit.dist = dot(e2, qvec) / det;
                hit.status = 1.f;
                hit.bc = vec3f16(1.0f - u - v, u, v);
            });
        });
    });

    return hit;
    // Pack vertices
    /*vec3f16 v0(_v0.x,_v0.y,_v0.z);
    vec3f16 v1(_v1.x,_v1.y,_v1.z);
    vec3f16 v2(_v2.x,_v2.y,_v2.z);

    RayHit16 hit;

    vec3f16 e1 = v1 - v0;
    vec3f16 e2 = v2 - v0;
    // Calculate planes normal vector
    vec3f16 pvec = cross(ray.dir, e2);
    Float16 det = dot(e1, pvec);

    // Ray is parallel to plane
    auto prevMask = !stack16().top();
    auto If0 = prevMask & abs(det) < 1e-8;
    if(all(If0))return hit;
    stack16().push(!If0);

    Float16 inv_det = Float16(1.0f) / det;
    vec3f16 tvec = ray.org - v0;
    Float16 u = dot(tvec, pvec) * inv_det;
    auto If1 = u < 0.0f | u > 1.0f;
    If1 = If0 & If1;
    if (all(If1))return hit;
    stack16().push(!If1);

    vec3f16 qvec = cross(tvec, e1);
    Float16 v = dot(ray.dir, qvec) * inv_det;
    auto If2 = v < 0.0f | u + v > 1.0f;
    If2 = If1 & If2;
    if (all(If2))return hit;
    stack16().push(!If2);

    hit.dist = dot(e2, qvec) * inv_det;
    hit.status = 1.f;
    hit.bc = vec3f16(1.0f-u-v,u,v);

    stack16().pop();
    stack16().pop();
    stack16().pop();

    return hit;*/
}


