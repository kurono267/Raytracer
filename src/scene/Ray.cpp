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
    hit.bc = vec3(u,v,1.0f-u-v);

    return hit;
}

