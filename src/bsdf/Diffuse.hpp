//
// Created by kurono267 on 2018-12-25.
//

#pragma once

#include "BxDF.hpp"

// Diffuse BRDF based at Oren Nayer model
class Diffuse : public BxDF {
public:
    Diffuse(const glm::vec3& _color, float _sigma);

    glm::vec3 f(const glm::vec3 &out, const glm::vec3 &in) const override;
protected:
    glm::vec3 color;
    float sigma;
    float A;
    float B;
};

