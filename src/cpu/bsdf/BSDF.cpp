//
// Created by kurono267 on 2018-12-25.
//

#include "BSDF.hpp"

BSDF::BSDF(const mango::sVertex &vertex, float _eta) : eta(_eta) {
	_world2local = glm::mat3(vertex.tangent, vertex.binormal, vertex.normal);
	_local2world = glm::transpose(_world2local);
	_normal = vertex.normal;
}

glm::vec3 BSDF::local2world(const glm::vec3 &v) const {
	return v * _local2world;
}

glm::vec3 BSDF::world2local(const glm::vec3 &v) const {
	return v * _world2local;
}

void BSDF::add(spBxDF b) {
	if (_numParts < MaxBxDFs) {
		_parts[_numParts] = b;
		_numParts++;
	}
}

uint32_t BSDF::numParts(BxDF::Type type) const {
	uint32_t parts = 0;
	for(int i = 0;i<_numParts;++i){
		if(_parts[i]->matchesFlags(type))++parts;
	}
	return parts;
}

glm::vec3 BSDF::f(const glm::vec3 &outWorld, const glm::vec3 &inWorld, BxDF::Type flags) const {
	glm::vec3 in = world2local(inWorld), out = world2local(outWorld);
	bool reflect = glm::dot(inWorld, _normal) * glm::dot(outWorld, _normal) > 0;
	glm::vec3 f(0.f);
	for (int i = 0; i < _numParts; ++i) {
		auto selectPart = _parts[i];
		if (selectPart->matchesFlags(flags) &&
			((reflect && (selectPart->type & BxDF::REFLECTION)) ||
			 (!reflect && (selectPart->type & BxDF::TRANSMISSION))))
			f += selectPart->f(out, in);
	}
	return f;
}

glm::vec3 BSDF::sample(const glm::vec3 &outWorld, const glm::vec2 &u, glm::vec3 &inWorld, float &pdf, BxDF::Type &sampledType,
					   BxDF::Type type) const {
	int matchingComps = numParts(type);
	pdf = 0.f;
	if (matchingComps == 0) {
		return glm::vec3(0.f);
	}
	int comp = std::min((int)std::floor(u[0] * matchingComps),matchingComps - 1);
	spBxDF bxdf = nullptr;
	int count = comp;
	for (int i = 0; i < _numParts; ++i) {
		if (_parts[i]->matchesFlags(type) && count-- == 0) {
			bxdf = _parts[i]; break;
		}
	}
	glm::vec2 uRemapped(u[0] * matchingComps - comp, u[1]);
	glm::vec3 wi, wo = world2local(outWorld);
	sampledType = (BxDF::Type)bxdf->type;
	glm::vec3 f = bxdf->sampled(wo, uRemapped, wi, pdf, sampledType);
	if (pdf == 0)
		return glm::vec3(0.f);
	inWorld = local2world(wi);
	if (!(bxdf->type & BxDF::SPECULAR) && matchingComps > 1) {
		for (int i = 0; i < _numParts; ++i) {
			if (_parts[i] != bxdf && _parts[i]->matchesFlags(type))
				pdf += _parts[i]->pdf(wo, wi);
		}
	}
	if (matchingComps > 1) pdf /= matchingComps;
	if (!(bxdf->type & BxDF::SPECULAR) && matchingComps > 1) {
		bool reflect = dot(inWorld, _normal) * dot(outWorld, _normal) > 0;
		f = glm::vec3(0.f); // TODO Don't sure what need
		for (int i = 0; i < _numParts; ++i)
			if (_parts[i]->matchesFlags(type) &&
				((reflect && (_parts[i]->type & BxDF::REFLECTION)) ||
				 (!reflect && (_parts[i]->type & BxDF::TRANSMISSION))))
				f += _parts[i]->f(wo, wi);
	}
	return f;
}

float BSDF::pdf(const glm::vec3 &outWorld, const glm::vec3 &inWorld, BxDF::Type flags) const {
	auto out = world2local(outWorld);
	auto in = world2local(inWorld);
	float _pdf = 0.f;
	float norm = 0.f;
	for (int i = 0; i < _numParts; ++i) {
		if (_parts[i]->matchesFlags(flags)) {
			_pdf += _parts[i]->pdf(out, in);
			++norm;
		}
	}
	if(norm != 0.0f)_pdf /= norm;
	return _pdf;
}
