//
// Created by kurono267 on 2018-12-14.
//

#include <thread>
#include "PathTracer.hpp"
#include "scene/bsdf/Diffuse.hpp"
#include "scene/bsdf/BSDF.hpp"
#include <random>

PathTracer::PathTracer(const spDevice& device,const spScene &scene) : _scene(scene), _device(device) {

}

void PathTracer::init() {
    //_bvh.run(_scene);
    _bvh.runLBVH(_scene);

    _texture = _device->createTexture(PT_WIDTH,PT_HEIGHT,1,Format::R32G32B32A32Sfloat,TextureType::Input);
    _frame[0] = std::make_shared<Image4f>(glm::ivec2(PT_WIDTH,PT_HEIGHT),1,glm::vec4(0.0f));
	_frame[1] = std::make_shared<Image4f>(glm::ivec2(PT_WIDTH,PT_HEIGHT),1,glm::vec4(0.0f));
    _textureCPU = _device->createBuffer(BufferType::CPU,MemoryType::HOST,PT_WIDTH*PT_HEIGHT*sizeof(glm::vec4),(void*)_frame[_frameID]->data().data());
}

spTexture PathTracer::getTexture() {
    return _texture;
}

void PathTracer::sync() {
    _textureCPU->set(PT_WIDTH*PT_HEIGHT*sizeof(glm::vec4),(void*)_frame[(_frameID+1)%2]->data().data());
    _texture->set(_textureCPU);
}

const uint32_t tileSize = 16;
const uint32_t threadsMax = 16;

void PathTracer::update(){
	if(_camera->isUpdated()){
		_frames = 0.0f;
		_camera->updateFinish();
	}
    auto start = std::chrono::system_clock::now();

    glm::ivec2 tilesRes(std::ceil((float)PT_WIDTH/(float)tileSize),std::ceil((float)PT_HEIGHT/(float)tileSize));
    std::atomic_int tileCurr = 0;
    std::atomic_int tileCount = tilesRes.x*tilesRes.y;

    std::mutex m;
    std::condition_variable cv;
    std::atomic_int counter = threadsMax;

    for(int th = 0;th<threadsMax;++th) {
        std::thread t([this,&counter,&m,&cv,&tileCurr,&tileCount,tilesRes]() {
            while(tileCurr < tileCount){
                int currTile = tileCurr++;
                glm::ivec2 tileStart(currTile%tilesRes.x,currTile/tilesRes.x);
                tileStart *= tileSize;

                computeTile(tileStart,_camera);
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

    if(!_camera->isUpdated())_frames++;
}

const int maxDepth = 4;

void PathTracer::computeTile(const glm::ivec2 &start, const mango::scene::spCamera &camera) {
    auto scene = _bvh.getScene();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.f, 1.f);

    // Tracing by 2x2 blocks for dFdx dFdy function support
    bool statusBlock[4];
    sVertex vertexBlock[4];
    spModel modelBlock[4];
    Ray     raysBlock[4];
    glm::vec2 ptSize(PT_WIDTH,PT_HEIGHT);

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
                    glm::vec3 image_point = normalized_i * _right +
                                            normalized_j * _up
                                            + _forward;
                    glm::vec3 ray_direction = normalize(image_point);

                    Ray r(_pos, ray_direction);
                    auto hit = _bvh.intersect(r);
                    auto inBlockID = yB*2+xB;
                    statusBlock[inBlockID] = hit.status;
                    if(hit.status){
                        vertexBlock[inBlockID] = _bvh.postIntersect(r, hit);
                        modelBlock[inBlockID] = scene->models()[hit.id0];
                        raysBlock[inBlockID] = r;
                    } else {
                    	glm::vec3 L(0.0f);
                    	for(auto light : _scene->lights()){
                    		L += light->le(r);
                    	}
						(*_frame[_frameID])(x,y) = glm::vec4(L,1.0f);
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
						dUVx *= ptSize;
						dUVy *= ptSize;

						// Shading
						auto model = modelBlock[yB*2+xB];
						auto material = model->material();
						auto bsdf = material->computeBSDF(vertex,dUVx,dUVy);

						auto ray = raysBlock[yB*2+xB];

						glm::vec3 out = -ray.dir;
						glm::vec3 pixelColor(0.f);
						glm::vec3 threshold(1.0f);
						for(int d = 0;d<maxDepth;++d) {
							for (auto light : _scene->lights()) {
								glm::vec3 in;
								float lightPdf;
								auto li = light->sampleLi(vertex, glm::vec2(dis(gen), dis(gen)), in, lightPdf);
								if(lightPdf == 0.0f)continue;
								auto shadowHit = _bvh.occluded(Ray(vertex.pos, in));
								if (shadowHit.status)continue;
								auto bsdfColor = bsdf->f(out, in);
								float bsdfPdf = bsdf->pdf(out, in);

								if(light->isDelta()){
									pixelColor += threshold * bsdfColor * li / lightPdf;
								} else {
									pixelColor += threshold * bsdfColor * li * powerHeuristic(1,lightPdf,1,bsdfPdf) / lightPdf;
									// MIS
									glm::vec3 inBSDF; BxDF::Type typeBSDF;
									glm::vec3 f = bsdf->sample(out,glm::vec2(dis(gen),dis(gen)),inBSDF,bsdfPdf,typeBSDF);
									if(bsdfPdf == 0.0f)continue;
									f *= std::abs(glm::dot(in, vertex.normal));
									bool isSpecular = typeBSDF & BxDF::SPECULAR;
									float weight = 1.f;
									if(!isSpecular){
										lightPdf = light->pdfLi(vertex, in);
										if(lightPdf == 0.0f)continue;
										weight = powerHeuristic(1, bsdfPdf, 1, lightPdf);
									}
									auto li = light->le(ray);
									if (!glm::all(glm::equal(li,glm::vec3(0.0f)))) pixelColor += f * li * weight / bsdfPdf;
								}
							}

							// Sampling BSDF
							glm::vec3 nextDir(0.0f); float nextPDF = 0.0f; BxDF::Type nextType;
							glm::vec3 bsdfColor = bsdf->sample(out,glm::vec2(dis(gen),dis(gen)),nextDir,nextPDF,nextType);

							if(glm::all(glm::equal(bsdfColor,glm::vec3(0.0f))) || nextPDF == 0.0f)break;

							threshold *= bsdfColor*std::abs(glm::dot(nextDir, vertex.normal))/nextPDF;
							ray = Ray(vertex.pos,nextDir);
							auto hit = _bvh.intersect(ray);
							if(hit.status){
								vertex = _bvh.postIntersect(ray,hit);
								model = scene->models()[hit.id0];
								material = model->material();
								bsdf = material->computeBSDF(vertex);
								out = -ray.dir;
							} else {
								break;
							}

						}

						if(_frames != 0.0f) {
							glm::vec3 prevFrame = (*_frame[(_frameID+1)%2])(x, y);
							prevFrame *= (float)_frames;
							prevFrame += pixelColor;
							prevFrame /= (float)_frames+1.f;

							(*_frame[_frameID])(x, y) = glm::vec4(prevFrame, 1.0f);
						} else {
							(*_frame[_frameID])(x, y) = glm::vec4(pixelColor, 1.0f);
						}
                    }
                }
            }

        }
    }
}

void PathTracer::run() {
	_renderThread = std::make_shared<std::thread>([this]() {
		while (_isRun) {
			if(!_isFinish){
				update();
				_frameID = (_frameID+1)%2;
				_isFinish = true;
			}
		}
	});
}

bool PathTracer::isUpdateFinish() {
	return _isFinish;
}

void PathTracer::nextFrame(const mango::scene::spCamera &camera) {
	_camera = camera;
	_right = camera->getRight();
	_forward = camera->getForward();
	_pos = camera->getPos();
	_up = glm::normalize(glm::cross(_right,_forward));
	_isFinish = false;
}

void PathTracer::finish() {
	_isRun = false;
	while(!_isFinish){}
}
