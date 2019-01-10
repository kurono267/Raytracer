//
// Created by kurono267 on 2019-01-04.
//

#pragma once

#include "Integrator.hpp"

using namespace mango::scene;

class BDPT : public Integrator {
	public:
		struct BDPTVertex {
			enum Type {
				Light,
				Camera,
				Surface
			};

			sVertex vertex;
			spLightSource light = nullptr;
			spBSDF bsdf = nullptr;

			float pdf;
			float pdfRev;
			glm::vec3 beta;

			bool endPoint = false;
			bool delta = false;
			Type type;

			Ray ray;
		};
	public:
		BDPT(const spDevice &device, const spScene &scene, int width, int height);

	protected:
		void computeTile(const glm::ivec2 &start, const mango::scene::spCamera &camera) override;
		void init() override;

		std::vector<BDPTVertex> createEyePath(const Ray& ray);
		std::vector<BDPTVertex> createLightPath();
		std::vector<BDPTVertex> walk(Ray ray, const int maxDepth, const BDPTVertex& first);

		std::random_device _rd;
		std::mt19937 _gen;
		std::uniform_real_distribution<> _dis;
};
