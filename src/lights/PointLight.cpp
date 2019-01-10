//
// Created by kurono267 on 2018-12-26.
//

#include "PointLight.hpp"

using namespace glm;

PointLight::PointLight(const glm::mat4 &light2world, const glm::vec3& I)
	: LightSource(Point, light2world), _light(I) {
	_pos = light2world*vec4(0.0f,0.0f,0.0f,1.0f);
}

glm::vec3 PointLight::sampleLi(const mango::sVertex &vertex, const glm::vec2 &sample, glm::vec3 &inWorld, float &pdf) {
	inWorld = normalize(_pos - vertex.pos);
	pdf = 1.f;
	return _light / length2(_pos - vertex.pos);
}

float PointLight::pdfLi(const mango::sVertex &vertex, const glm::vec3 &inWorld) {
	return 1.0f;
}

vec3 PointLight::power() {
	return 4 * (float)M_PI * _light;
}


