//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include "../scene/Scene.hpp"
#include "../scene/BVH.hpp"
#include <thread>

const int PT_WIDTH = 128*5;
const int PT_HEIGHT = 72*5;

class PathTracer {
public:
    PathTracer(const spDevice& device,const spScene& scene);

    void init();

    void run();
    bool isUpdateFinish();
    void nextFrame(const mango::scene::spCamera& camera);
    void finish();

    spTexture getTexture();

    void sync();
private:
	void update();
    void computeTile(const glm::ivec2& start, const mango::scene::spCamera& camera);
private:
    spDevice _device;

    spScene _scene;
    BVH     _bvh;

    int _frameID = 0;
    spImage4f _frame[2];
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
};

typedef std::shared_ptr<PathTracer> spPathTracer;

