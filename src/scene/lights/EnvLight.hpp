//
// Created by kurono267 on 2018-12-26.
//

#pragma once

#include "LightSource.hpp"
#include "scene/samplers/Distributions.hpp"

class EnvLight : public LightSource {
	public:
		EnvLight(const spImage4f& image, const glm::vec3& light);

		glm::vec3 sampleLi(const mango::sVertex &vertex, const glm::vec2 &sample, glm::vec3 &inWorld, float &pdf) override;
		float pdfLi(const mango::sVertex &vertex, const glm::vec3 &inWorld) override;

		glm::vec3 power() override;
	private:
		spImage4f _image;
		glm::vec3 _light;
		std::unique_ptr<Distribution2D> _dist;
};

