//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include <mango.hpp>

struct BVHNode {
    BVHNode() : data(-1){}
    BVHNode(const BVHNode& n) : min(n.min),max(n.max),data(n.data) {}

    // Alligned BVH Node
    glm::vec4 min; // xyz min, w split
    glm::vec4 max; // xyz max, w 1 if leaf

    // Data changes by leaf or not current node
    // If Leaf x,y,z,w index of triangle
    // If Not leaf x,y r leaf, l leaf, z - meshID, w - depth
    glm::ivec4 data;
};

struct BBox {
    BBox();
    BBox(const BVHNode& node);
    BBox(const BBox& a);

    void expand(const glm::vec3& _min,const glm::vec3& _max);
    void expand(const glm::vec3& _p);

    uint32_t maxDim();

    glm::vec3 min;
    glm::vec3 max;
};
