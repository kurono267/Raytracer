//
// Created by kurono267 on 2018-12-25.
//

#pragma once

#include <mango.hpp>

#include "Samples.hpp"

class BxDF {
    public:
		enum Type {
            REFLECTION = 1 << 0,
            TRANSMISSION = 1 << 1,
            DIFFUSE = 1 << 2,
            GLOSSY = 1 << 3,
            SPECULAR = 1 << 4,
            ALL = DIFFUSE | GLOSSY | SPECULAR | REFLECTION | TRANSMISSION
        };
    public:
		BxDF(const uint32_t& _type) : type(_type) {}
        virtual ~BxDF() = default;

		bool matchesFlags(Type t) const { return (type & t) == type; }

        virtual glm::vec3 f(const glm::vec3& out, const glm::vec3& in) = 0;
        virtual glm::vec3 sampled(const glm::vec3& out, const glm::vec2& sample, glm::vec3& in, float& pdf, Type& type);

        virtual float pdf(const glm::vec3& out, const glm::vec3& in);

        const uint32_t type;
};

typedef std::shared_ptr<BxDF> spBxDF;

inline float cosTheta(const glm::vec3& w) { return w.z; }
inline float cos2Theta(const glm::vec3& w) { return w.z * w.z; }
inline float absCosTheta(const glm::vec3& w) { return std::abs(w.z); }

inline float sin2Theta(const glm::vec3& w) { return std::max((float)0, (float)1 - cos2Theta(w)); }
inline float sinTheta(const glm::vec3& w) { return std::sqrt(sin2Theta(w)); }

inline float tanTheta(const glm::vec3& w) { return sinTheta(w) / cosTheta(w); }
inline float tan2Theta(const glm::vec3& w) { return sin2Theta(w) / cos2Theta(w); }

inline float clamp(float v, float _min, float _max) {
	return std::max(std::min(v,_max),_min);
}

inline float cosPhi(const glm::vec3 &w) {
    float sTheta = sinTheta(w);
    return (sTheta == 0) ? 1 : clamp(w.x / sTheta, -1, 1);
}
inline float sinPhi(const glm::vec3 &w) {
    float sTheta = sinTheta(w);
    return (sTheta == 0) ? 0 : clamp(w.y / sTheta, -1, 1);
}

inline float cos2Phi(const glm::vec3 &w) {
    return cosPhi(w) * cosPhi(w);
}
inline float sin2Phi(const glm::vec3 &w) {
    return sinPhi(w) * sinPhi(w);
}

inline bool sameHemisphere(const glm::vec3& w, const glm::vec3& wp) { return w.z * wp.z > 0; }
