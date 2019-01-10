//
// Created by kurono267 on 2019-01-10.
//

#include "PathTracer16.hpp"

PathTracer16::PathTracer16(const spDevice &device, const spScene &scene, int width, int height) : Integrator(device, scene,
																										 width,
																										 height) {}

void PathTracer16::init() {

}

const int maxDepth = 4;

void PathTracer16::computeTile(const glm::ivec2 &start, const mango::scene::spCamera &camera) {
	auto scene = _bvh.getScene();

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0.f, 1.f);

	glm::vec2 ptSize(_width,_height);
	int blockSize = 4;

	vec3f16 cameraPos(_pos.x,_pos.y,_pos.z);
	clearMask16();

	for(int yBlock = start.y;yBlock<start.y+tileSize;yBlock+=blockSize){
		if(yBlock >= _height)break;
		for(int xBlock = start.x;xBlock<start.x+tileSize;xBlock+=blockSize){
			if(xBlock >= _width)break;

			// Fill blocks Rays
			Ray16 ray16;
			ray16.org = cameraPos;
			for(int yB = 0;yB<blockSize;++yB){
				int y = yBlock+yB;
				if(y >= _height)break;
				for(int xB = 0;xB<blockSize;++xB){
					int x = xBlock+xB;
					if(x >= _width)break;

					// TODO can optimization it's part
					float normalized_i = ((float) x / (float) _width) - 0.5f;
					float normalized_j = ((float) y / (float) _height) - 0.5f;
					normalized_j = -normalized_j;
					normalized_j *= ((float) _height / (float) _width);
					glm::vec3 image_point = normalized_i * _right +
											normalized_j * _up
											+ _forward;
					glm::vec3 ray_direction = normalize(image_point);

					ray16.dir.x[yB*blockSize+xB] = ray_direction.x;
					ray16.dir.y[yB*blockSize+xB] = ray_direction.y;
					ray16.dir.z[yB*blockSize+xB] = ray_direction.z;
				}
			}

			RayHit16 hit = _bvh.intersect16(ray16);

			clearMask16();
			vec3f16 color = vec3f16(0.5f);
			If(hit.status != 0.f,[&](){
				color = vec3f16(/*hit.dist*/1.f);
			}).Else([&](){
				color = vec3f16(0.0f);
			});

			for(int yB = 0;yB<blockSize;++yB) {
				int y = yBlock + yB;
				if (y >= _height)break;
				for (int xB = 0; xB < blockSize; ++xB) {
					int x = xBlock + xB;
					if (x >= _width)break;

					int id = yB*blockSize+xB;

					(*_frame[_frameID])(x, y) = glm::vec4(color.x[id],color.y[id],color.z[id],1.f);
				}
			}
		}
	}
}

