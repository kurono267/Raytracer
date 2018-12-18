//
// Created by kurono267 on 2018-12-14.
//

#include "BBox.hpp"

BBox::BBox() : min(std::numeric_limits<float>::infinity()), max(-std::numeric_limits<float>::infinity()) {}
BBox::BBox(const BBox& a) : min(a.min), max(a.max) {}

void BBox::expand(const glm::vec3& _min,const glm::vec3& _max){
    min = glm::min(_min,min);
    max = glm::max(_max,max);
}
void BBox::expand(const glm::vec3& _p){
    min = glm::min(_p,min);
    max = glm::max(_p,max);
}

uint32_t BBox::maxDim(){
    glm::vec3 axisSize = max-min;
    if(axisSize.x <= axisSize.y){
        if(axisSize.y >= axisSize.z)return 1; // Y axis
    } else {
        if(axisSize.x >= axisSize.z)return 0; // X axis
    }
    return 2; // Z axis Z more then other
}