//
// Created by kurono267 on 2018-12-14.
//

#pragma once

#include "../scene/Scene.hpp"
#include "../scene/BVH.hpp"

const int PT_WIDTH = 128*5;
const int PT_HEIGHT = 72*5;

class PathTracer {
public:
    PathTracer(const spDevice& device,const spScene& scene);

    void init();

    spTexture getTexture();

    void update(const mango::scene::spCamera& camera);

    void sync();
private:
    void computeTile(const glm::ivec2& start, const mango::scene::spCamera& camera);
private:
    spDevice _device;

    spScene _scene;
    BVH     _bvh;

    spImage4f _frame;
    spTexture _texture;
    spBuffer  _textureCPU;
};

typedef std::shared_ptr<PathTracer> spPathTracer;

