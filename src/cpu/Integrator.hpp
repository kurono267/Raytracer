//
// Created by kurono267 on 2019-01-04.
//

#pragma once

#include "scene/Scene.hpp"
#include "scene/BVH.hpp"
#include "bsdf/Diffuse.hpp"
#include "bsdf/BSDF.hpp"
#include <random>
#include <thread>

class Integrator {
	public:
		Integrator(const spDevice& device,const spScene& scene, int width, int height);

		void run();
		bool isUpdateFinish();
		void nextFrame(const mango::scene::spCamera& camera);
		void finish();

		spTexture getTexture();

		void sync();
	protected:
		void update();
		virtual void computeTile(const glm::ivec2& start, const mango::scene::spCamera& camera) = 0;
		virtual void init() = 0;
	protected:
		spDevice _device;

		spScene _scene;
		BVH     _bvh;

		int _frameID = 0;
		spImage4f _frame[2];
		int _width;
		int _height;

		spTexture _texture;
		spBuffer  _textureCPU;

		mango::scene::spCamera _camera;
		glm::vec3 _right;
		glm::vec3 _forward;
		glm::vec3 _pos;
		glm::vec3 _up;

		float _frames;

		std::shared_ptr<std::thread> _renderThread;
		std::atomic_bool _isFinish = true;
		std::atomic_bool _isRun = true;

	const uint32_t tileSize = 16;
	const uint32_t threadsMax = 16;
};

typedef std::shared_ptr<Integrator> spIntegrator;
