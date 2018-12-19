//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include "BBox.hpp"

struct Ray {
    Ray(const glm::vec3& _org, const glm::vec3& _dir) : org(_org),dir(_dir){
        invdir = 1.0f/dir;
        for(int i = 0;i<3;++i){
            if(dir[i] == 0.0f)invdir[i] = 0.0f;
        }
    }

    glm::vec3 org;
    glm::vec3 dir;
    glm::vec3 invdir;
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
