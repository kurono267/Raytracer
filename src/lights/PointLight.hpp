//
// Created by kurono267 on 2018-12-26.
//

#pragma once

#include "LightSource.hpp"

class PointLight : public LightSource {
	public:
		PointLight(const glm::mat4 &light2world, const glm::vec3& I);
		~PointLight() override = default;

		glm::vec3 sampleLi(const mango::sVertex &vertex, const glm::vec2 &sample, glm::vec3 &inWorld, float &pdf) override;
		float pdfLi(const mango::sVertex &vertex, const glm::vec3 &inWorld) override;

		glm::vec3 power() override;
	private:
			glm::vec3 _light;
			glm::vec3 _pos;
};
