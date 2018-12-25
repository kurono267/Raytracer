//
// Created by kurono267 on 2018-12-14.
//

#include <thread>
#include "PathTracer.hpp"
#include "bsdf/Diffuse.hpp"
#include "bsdf/BSDF.hpp"
#include <random>

PathTracer::PathTracer(const spDevice& device,const spScene &scene) : _scene(scene), _device(device) {

}

void PathTracer::init() {
    //_bvh.run(_scene);
    _bvh.runLBVH(_scene);

    _texture = _device->createTexture(PT_WIDTH,PT_HEIGHT,1,Format::R32G32B32A32Sfloat,TextureType::Input);
    _frame = std::make_shared<Image4f>(glm::ivec2(PT_WIDTH,PT_HEIGHT),1,glm::vec4(0.0f));
    _textureCPU = _device->createBuffer(BufferType::CPU,MemoryType::HOST,PT_WIDTH*PT_HEIGHT*sizeof(glm::vec4),(void*)_frame->data().data());
}

spTexture PathTracer::getTexture() {
    return _texture;
}

void PathTracer::sync() {
    _textureCPU->set(PT_WIDTH*PT_HEIGHT*sizeof(glm::vec4),(void*)_frame->data().data());
    _texture->set(_textureCPU);
}

const uint32_t tileSize = 16;
const uint32_t threadsMax = 16;

void PathTracer::update(const mango::scene::spCamera& camera){
    auto start = std::chrono::system_clock::now();

    glm::ivec2 tilesRes(std::ceil((float)PT_WIDTH/(float)tileSize),std::ceil((float)PT_HEIGHT/(float)tileSize));
    std::atomic_int tileCurr = 0;
    std::atomic_int tileCount = tilesRes.x*tilesRes.y;

    std::mutex m;
    std::condition_variable cv;
    std::atomic_int counter = threadsMax;

    for(int th = 0;th<threadsMax;++th) {
        std::thread t([this,&counter,&m,&cv,&tileCurr,&tileCount,tilesRes,camera]() {
            while(tileCurr < tileCount){
                int currTile = tileCurr++;
                glm::ivec2 tileStart(currTile%tilesRes.x,currTile/tilesRes.x);
                tileStart *= tileSize;

                computeTile(tileStart,camera);
            }

            std::lock_guard<std::mutex> lk(m);
            counter--;
            cv.notify_all();
        });
        t.detach();
    }

    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [&counter](){ return counter == 0; });

    auto end = std::chrono::system_clock::now();
    float frameTime = std::chrono::duration_cast<std::chrono::duration<float> >(end-start).count();
    std::cout << "PT Time:" << frameTime*1000.0f << " ms" << std::endl;
    std::cout << "FPS: " << 1.0f/frameTime << std::endl;
}

void PathTracer::computeTile(const glm::ivec2 &start, const mango::scene::spCamera &camera) {
    auto right = camera->getRight();
    auto forward = camera->getForward();
    auto pos = camera->getPos();
    auto up = camera->getUp();

    auto scene = _bvh.getScene();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.f, 1.f);

    /*
     if (hit.status) {
                        sVertex hitVertex = _bvh.postIntersect(r, hit);

                        // Shading
                        auto model = scene->models()[hit.id0];
                        auto material = model->material();

                        auto diffuseTexture = material->getDiffuseTexture();
                        auto repeatUV = glm::fract(hitVertex.uv);
                        glm::vec3 diffColor = diffuseTexture ? (*diffuseTexture)(repeatUV, 3) : glm::vec3(255.0f);
                        diffColor /= 255.0f;

                        float light = std::max(0.0f, glm::dot(hitVertex.normal, glm::vec3(0.0, 1.0f, 0.0f)));

                        glm::vec4 outColor(diffColor * material->getDiffuseColor() * light, 1.0f);

                        //(*_frame)(x,y) = glm::vec4(hitVertex.uv.x,hitVertex.uv.y,1.0f,1.0f);
                        (*_frame)(x, y) = outColor;
                        //(*_frame)(x,y) = glm::vec4(glm::vec3(std::max(0.0f,glm::dot(hitVertex.normal,glm::vec3(0.0,1.0f,0.0f)))),1.0f);
                    } else {
                        (*_frame)(x, y) = glm::vec4(0.0f);
                    }
     */

    // Tracing by 2x2 blocks for dFdx dFdy function support
    bool statusBlock[4];
    sVertex vertexBlock[4];
    spModel modelBlock[4];
    Ray     raysBlock[4];
    glm::vec2 ptSize(PT_WIDTH,PT_HEIGHT);

    glm::vec3 lightPos(0.0f,10.0f,0.0f);
    //glm::vec3 lightDir(0.0f,1.0f,0.0f);

    for(int yBlock = start.y;yBlock<start.y+tileSize;yBlock+=2){
        if(yBlock >= PT_HEIGHT)break;
        for(int xBlock = start.x;xBlock<start.x+tileSize;xBlock+=2){
            if(xBlock >= PT_WIDTH)break;

            // Intersection Block
            for(int yB = 0;yB<2;++yB) {
                int y = yBlock+yB;
                if(y >= PT_HEIGHT)break;
                for(int xB = 0;xB<2;++xB) {
                    int x = xBlock+xB;
                    if(x >= PT_WIDTH)break;
                    float normalized_i = ((float) x / (float) PT_WIDTH) - 0.5f;
                    float normalized_j = ((float) y / (float) PT_HEIGHT) - 0.5f;
                    normalized_j = -normalized_j;
                    normalized_j *= ((float) PT_HEIGHT / (float) PT_WIDTH);
                    glm::vec3 image_point = normalized_i * right +
                                            normalized_j * up
                                            + forward;
                    glm::vec3 ray_direction = normalize(image_point);

                    Ray r(pos, ray_direction);
                    auto hit = _bvh.intersect(r);
                    auto inBlockID = yB*2+xB;
                    statusBlock[inBlockID] = hit.status;
                    if(hit.status){
                        vertexBlock[inBlockID] = _bvh.postIntersect(r, hit);
                        modelBlock[inBlockID] = scene->models()[hit.id0];
                        raysBlock[inBlockID] = r;
                    }
                }
            }

            for(int yB = 0;yB<2;++yB) {
                int y = yBlock+yB;
                if(y >= PT_HEIGHT)break;
                for(int xB = 0;xB<2;++xB) {
                    int x = xBlock + xB;
                    if (x >= PT_WIDTH)break;

                    if(statusBlock[yB*2+xB]){
						auto vertex = vertexBlock[yB*2+xB];

						auto dUVx = vertexBlock[yB*2+1].uv-vertexBlock[yB*2+0].uv;
						auto dUVy = vertexBlock[1*2+xB].uv-vertexBlock[xB].uv;

						glm::vec3 out = -raysBlock[yB*2+xB].dir;
						auto in = normalize(lightPos-vertex.pos);

						dUVx *= ptSize;
						dUVy *= ptSize;

                        // Shading
                        auto model = modelBlock[yB*2+xB];
                        auto material = model->material();
                        auto bsdf = material->computeBSDF(vertex,dUVx,dUVy);

                        //(*_frame)(x,y) = glm::vec4(hitVertex.uv.x,hitVertex.uv.y,1.0f,1.0f);

                        //glm::vec3 in; float pdf; BxDF::Type type;
                        //auto bxdfColor = diffuseTest.sampled(outLocal,glm::vec2(dis(gen),dis(gen)),in,pdf,type);
                        auto bxdfColor = bsdf->f(out,in);
                        float pdf = bsdf->pdf(out,in);

                        (*_frame)(x, y) = glm::vec4(bxdfColor*pdf*10.0f,1.0f);
                    } else {
                        (*_frame)(x, y) = glm::vec4(0.0f);
                    }
                }
            }

        }
    }
}
