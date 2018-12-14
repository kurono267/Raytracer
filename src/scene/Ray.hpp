//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include "BBox.hpp"

struct Ray {
    glm::vec3 org;
    glm::vec3 dir;
};

struct RayHit {
    RayHit();

    bool status;
    float dist;
    int id0;
    int id1;
};

RayHit intersectBBox(const Ray& ray,const BBox& box);
RayHit intersectTriangle(const Ray& ray,const glm::vec3& v0,const glm::vec3& v1,const glm::vec3& v2);
