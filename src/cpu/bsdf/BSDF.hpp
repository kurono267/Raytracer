//
// Created by kurono267 on 2018-12-25.
//

#pragma once

#include "BxDF.hpp"

constexpr uint32_t MaxBxDFs = 10;

class BSDF {
	public:
		BSDF(const mango::sVertex& vertex,float eta);

		glm::vec3 local2world(const glm::vec3& v) const ;
		glm::vec3 world2local(const glm::vec3& v) const ;

		void add(spBxDF b);
		uint32_t numParts(BxDF::Type type = BxDF::ALL) const ;

		glm::vec3 f(const glm::vec3 &outWorld, const glm::vec3 &inWorld, BxDF::Type flags = BxDF::ALL) const;
		glm::vec3 sample(const glm::vec3 &outWorld, const glm::vec2 &u, glm::vec3& inWorld, float& pdf, BxDF::Type& sampledType, BxDF::Type type = BxDF::ALL) const;
		float pdf(const glm::vec3 &outWorld, const glm::vec3 &inWorld, BxDF::Type flags = BxDF::ALL) const;

		const float eta;
	private:
		spBxDF _parts[MaxBxDFs];
		uint32_t _numParts = 0;
	private:
		glm::mat3 _world2local;
		glm::mat3 _local2world;
		glm::vec3 _normal;
};

typedef std::shared_ptr<BSDF> spBSDF;

