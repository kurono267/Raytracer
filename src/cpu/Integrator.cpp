//
// Created by kurono267 on 2019-01-04.
//

#include "Integrator.hpp"

Integrator::Integrator(const spDevice& device,const spScene &scene, int width, int height)
: _scene(scene), _device(device),_width(width),_height(height) {
	_bvh.runLBVH(_scene);

	_texture = _device->createTexture(_width,_height,1,Format::R32G32B32A32Sfloat,TextureType::Input);
	_frame[0] = std::make_shared<Image4f>(glm::ivec2(_width,_height),1,glm::vec4(0.0f));
	_frame[1] = std::make_shared<Image4f>(glm::ivec2(_width,_height),1,glm::vec4(0.0f));
	_textureCPU = _device->createBuffer(BufferType::CPU,MemoryType::HOST,_width*_height*sizeof(glm::vec4),(void*)_frame[_frameID]->data().data());
}

spTexture Integrator::getTexture() {
	return _texture;
}

void Integrator::sync() {
	_textureCPU->set(_width*_height*sizeof(glm::vec4),(void*)_frame[(_frameID+1)%2]->data().data());
	_texture->set(_textureCPU);
}

void Integrator::update(){
	if(_camera->isUpdated()){
		_frames = 0.0f;
		_camera->updateFinish();
	}
	auto start = std::chrono::system_clock::now();

	glm::ivec2 tilesRes(std::ceil((float)_width/(float)tileSize),std::ceil((float)_width/(float)tileSize));
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
	std::cout << "Integrator Time:" << frameTime*1000.0f << " ms" << std::endl;
	std::cout << "FPS: " << 1.0f/frameTime << std::endl;

	if(!_camera->isUpdated())_frames++;
}

void Integrator::run() {
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

bool Integrator::isUpdateFinish() {
	return _isFinish;
}

void Integrator::nextFrame(const mango::scene::spCamera &camera) {
	_camera = camera;
	_right = camera->getRight();
	_forward = camera->getForward();
	_pos = camera->getPos();
	_up = glm::normalize(glm::cross(_right,_forward));
	_isFinish = false;
}

void Integrator::finish() {
	_isRun = false;
	while(!_isFinish){}
}
