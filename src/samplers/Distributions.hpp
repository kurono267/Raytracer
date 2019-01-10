//
// Created by kurono267 on 2018-12-26.
//

#pragma once

#include <mango.hpp>

#include "scene/Common.hpp"

class Distribution1D {
public:
	Distribution1D(const std::vector<float> &data);

	int count() const ;

	float sampleContinuous(float u, float &pdf, int *off = nullptr) const ;
	int sampleDiscrete(float u, float& pdf, float *uRemapped = nullptr) const ;

	float discretePDF(int index) const ;

	std::vector<float> func;
	std::vector<float> cdf;
	float funcInt;
};

class Distribution2D {
public:
	Distribution2D(const std::vector<float>& data, int width, int height);

	glm::vec2 sampleContinuous(const glm::vec2 &u, float& pdf) const ;
	float pdf(const glm::vec2 &p) const ;
private:
	std::vector<std::unique_ptr<Distribution1D>> pConditionalV;
	std::unique_ptr<Distribution1D> pMarginal;
};
