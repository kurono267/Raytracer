//
// Created by kurono267 on 2018-12-25.
//

#include "Diffuse.hpp"

Diffuse::Diffuse(const glm::vec3 &_color, float _sigma) : BxDF(Type::DIFFUSE | Type::REFLECTION), color(_color),sigma(_sigma) {
    float sigma2 = sigma*sigma;
    A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
    B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

glm::vec3 Diffuse::f(const glm::vec3 &out, const glm::vec3 &in) const {
    float sinThetaI = sinTheta(in); float sinThetaO = sinTheta(out);
    float maxCos = 0;
    if (sinThetaI > 1e-4 && sinThetaO > 1e-4) {
        float sinPhiI = sinPhi(in), cosPhiI = cosPhi(in);
        float sinPhiO = sinPhi(out), cosPhiO = cosPhi(out);
        float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
        maxCos = std::max((float)0, dCos);
    }

	float sinAlpha, tanBeta;
    if (absCosTheta(in) > absCosTheta(out)) {
        sinAlpha = sinThetaO;
        tanBeta = sinThetaI / absCosTheta(in);
    } else {
        sinAlpha = sinThetaI;
        tanBeta = sinThetaO / absCosTheta(out);
    }
    return color * INV_PI * (A + B * maxCos * sinAlpha * tanBeta);
}
