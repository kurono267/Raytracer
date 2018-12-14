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
    auto right = camera->getRight();
    auto forward = camera->getForward();
    auto pos = camera->getPos();
    auto up = camera->getUp();

    for(int y = 0;y<PT_HEIGHT;++y){
        for(int x = 0;x<PT_WIDTH;++x){
            float normalized_i = ((float)x / (float)PT_WIDTH) - 0.5f;
            float normalized_j = ((float)y / (float)PT_HEIGHT) - 0.5f;
            normalized_j = -normalized_j;
            normalized_j *= ((float)PT_HEIGHT/(float)PT_WIDTH);
            glm::vec3 image_point = normalized_i*right +
                               normalized_j*up
                               + forward;
            glm::vec3 ray_direction = normalize(image_point);

            Ray r;
            r.org = pos;
            r.dir = ray_direction;

            RayHit hit = _bvh.intersect(r);
            if(hit.status){
                auto point = r.org+r.dir*hit.dist;
                point /= 20.0f;
                (*_frame)(x,y) = glm::vec4(glm::pow(point*0.5f+0.5f,glm::vec3(1.0f/2.4f)),1.0f);
            } else {
                (*_frame)(x,y) = glm::vec4(0.0f);
            }
            //(*_frame)(x,y) = glm::vec4(ray_direction,1.0f);
        }
    }
    _time += 0.01f;
    if(_time >= 1.0f)_time = 0.0f;
}
