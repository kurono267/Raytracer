//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include "Integrator.hpp"

class PathTracer : public Integrator {
	public:
		PathTracer(const spDevice& device,const spScene& scene, int width, int height);
	protected:
		void init() final;
		void computeTile(const glm::ivec2& start, const mango::scene::spCamera& camera) final;
};

typedef std::shared_ptr<PathTracer> spPathTracer;

