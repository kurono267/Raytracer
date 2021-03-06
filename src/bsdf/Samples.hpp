//
// Created by kurono267 on 2018-12-25.
//

#pragma once

#include <mango.hpp>
#include "scene/Common.hpp"

constexpr float INV_PI = 1.0f/(float)M_PI;
constexpr float INV_2PI = 1.0f/(2.f*(float)M_PI);
constexpr float PI_OVER_2 = (float)M_PI/2.f;
constexpr float PI_OVER_4 = PI_OVER_2/2.f;

inline glm::vec2 concentricSampleDisk(const glm::vec2& u) {
	glm::vec2 uOffset = 2.f * u - glm::vec2(1.f);
	if (uOffset.x == 0 && uOffset.y == 0)
		return glm::vec2(0.f);
	float theta, r;
	if (std::abs(uOffset.x) > std::abs(uOffset.y)) {
		r = uOffset.x;
		theta = PI_OVER_4 * (uOffset.y / uOffset.x);
	} else {
		r = uOffset.y;
		theta = PI_OVER_2 - PI_OVER_4 * (uOffset.x / uOffset.y);
	}
	return r * glm::vec2(std::cos(theta), std::sin(theta));
}

inline glm::vec3 cosineSampleHemisphere(const glm::vec2& u) {
	glm::vec2 d = concentricSampleDisk(u);
	float z = std::sqrt(std::max(0.f, 1.f - d.x * d.x - d.y * d.y));
	return glm::vec3(d.x, z, d.y);
}

inline float sphericalTheta(const glm::vec3& v) {
	return std::acos(clamp(v.y, -1.f, 1.f));
}

inline float sphericalPhi(const glm::vec3& v) {
	float p = std::atan2(-v.z, -v.x);
	return (p < 0.f) ? (p + 2.f * (float)M_PI) : p;
}

inline glm::vec3 sphericalDirection(float sinTheta, float cosTheta, float phi) {
	return glm::vec3(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
					cosTheta);
}
