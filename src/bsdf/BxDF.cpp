//
// Created by kurono267 on 2018-12-25.
//


#include "BxDF.hpp"

glm::vec3 BxDF::sampled(const glm::vec3 &out, const glm::vec2 &sample, glm::vec3 &in, float &_pdf, BxDF::Type &type) const {
	in = cosineSampleHemisphere(sample);
	if (out.z < 0) in.z *= -1;
	_pdf = pdf(out, in);
	return f(out, in);
}

float BxDF::pdf(const glm::vec3 &out, const glm::vec3 &in) const {
	return sameHemisphere(out, in) ? absCosTheta(in) * INV_PI : 0;
}

