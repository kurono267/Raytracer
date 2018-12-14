//
// Created by kurono267 on 2018-12-14.
//

#include "PathTracer.hpp"

PathTracer::PathTracer(const spDevice& device,const spScene &scene) : _scene(scene), _device(device) {

}

void PathTracer::init() {
    _bvh.run(_scene);

    _texture = _device->createTexture(PT_WIDTH,PT_HEIGHT,1,Format::R32G32B32A32Sfloat,TextureType::Input);
    _frame = std::make_shared<Image4f>(glm::ivec2(PT_WIDTH,PT_HEIGHT),glm::vec4(0.0f));
    _textureCPU = _device->createBuffer(BufferType::CPU,MemoryType::HOST,PT_WIDTH*PT_HEIGHT*sizeof(glm::vec4),(void*)_frame->data().data());
}

spTexture PathTracer::getTexture() {
    return _texture;
}

void PathTracer::sync() {
    _textureCPU->set(PT_WIDTH*PT_HEIGHT*sizeof(glm::vec4),(void*)_frame->data().data());
    _texture->set(_textureCPU);
}

void PathTracer::update(const mango::scene::spCamera& camera){
    for(int y = 0;y<PT_HEIGHT;++y){
        for(int x = 0;x<PT_WIDTH;++x){
            (*_frame)(x,y) = glm::vec4(_time,_time,_time,1.0f);
        }
    }
    _time += 0.01f;
    if(_time >= 1.0f)_time = 0.0f;
}
