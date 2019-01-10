//
// Created by kurono267 on 2019-01-10.
//

#pragma once

#include "Integrator.hpp"

class PathTracer16 : public Integrator {
public:
	PathTracer16(const spDevice& device,const spScene& scene, int width, int height);
protected:
	void init() final;
	void computeTile(const glm::ivec2& start, const mango::scene::spCamera& camera) final;
};

typedef std::shared_ptr<PathTracer16> spPathTracer16;

