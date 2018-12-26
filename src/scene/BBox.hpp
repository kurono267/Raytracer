//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include <mango.hpp>

struct BBox {
    BBox();
    BBox(const BBox& a);

    void expand(const glm::vec3& _min,const glm::vec3& _max);
    void expand(const glm::vec3& _p);

    uint32_t maxDim();

    union {
        glm::vec3 bounds[2];
        struct {
            glm::vec3 min;
            glm::vec3 max;
        };
    };
};

struct BVHNode {
    BVHNode(){
        for(int i = 0;i<4;++i)triIds[i] = -1;
        for(int i = 0;i<4;++i)modelIds[i] = -1;
        isLeaf = false;
    }
    BVHNode(const BVHNode& n) : box(n.box),data(n.data),isLeaf(n.isLeaf),split(n.split) {
        for(int i = 0;i<4;++i)modelIds[i] = n.modelIds[i];
    }

    // Alligned BVH Node
    BBox box;

    // Data changes by leaf or not current node
    // If Leaf x,y,z,w index of triangle
    // If Not leaf x,y r leaf, l leaf, z - meshID, w - depth
    union {
        int32_t triIds[4];
        struct {
            int32_t right;
            int32_t left;
            int32_t meshID;
            int32_t depth;
        } data;
    };
    int32_t modelIds[4];

    float split;
    bool isLeaf;
};
