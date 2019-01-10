//
// Created by kurono267 on 2019-01-04.
//

#include "BDPT.hpp"

BDPT::BDPT(const spDevice &device, const spScene &scene, int width, int height) : Integrator(device, scene, width,
																							 height), _gen(_rd()), _dis(0.f,1.f) {

}

const int maxDepth = 4;

void BDPT::computeTile(const glm::ivec2 &start, const mango::scene::spCamera &camera) {
	for(int y = start.y;y<start.y+tileSize;++y){
		if(y >= _height)break;
		for(int x = start.x;x<start.x+tileSize;++x){
			if(x >= _width)break;

			float normalized_i = ((float) x / (float) _width) - 0.5f;
			float normalized_j = ((float) y / (float) _height) - 0.5f;
			normalized_j = -normalized_j;
			normalized_j *= ((float) _height / (float) _width);
			glm::vec3 image_point = normalized_i * _right +
									normalized_j * _up
									+ _forward;
			glm::vec3 ray_direction = normalize(image_point);

			Ray r(_pos, ray_direction);

		}
	}
}

void BDPT::init() {

}

std::vector<BDPT::BDPTVertex> BDPT::createEyePath(const Ray &ray) {
	BDPTVertex vertex;
	vertex.ray = ray;
	vertex.pdf = 1.f;
	vertex.beta = glm::vec3(1.f);
	vertex.type = BDPTVertex::Camera;
	return walk(ray,maxDepth-1,vertex);
}

std::vector<BDPT::BDPTVertex> BDPT::walk(Ray ray, const int maxDepth, const BDPTVertex& first) {
	std::vector<BDPTVertex> path;
	path.push_back(first);
	if (maxDepth == 0) return path;
	int bounces = 0;
	// Declare variables for forward and reverse probability densities
	float pdfFwd = 1.f, pdfRev = 0;
	glm::vec3 beta(1.f);
	while (true) {
		BDPTVertex point;

		auto hit = _bvh.intersect(ray);
		if (!hit.status) {
			point.type = BDPTVertex::Light;
			point.endPoint = true;
			point.pdf = pdfFwd;
			point.ray = ray;
			path.push_back(point);

			++bounces;
			break;
		}

		auto vertex = _bvh.postIntersect(ray,hit);

		auto model = _scene->models()[hit.id0];
		auto material = model->material();
		auto bsdf = material->computeBSDF(vertex);

		// Initialize _vertex_ with surface intersection information
		point.vertex = vertex;
		point.type = BDPTVertex::Surface;
		point.pdf = pdfFwd;
		point.beta = beta;
		if (++bounces >= maxDepth) {
			path.push_back(point);
			break;
		}

		glm::vec3 wi, wo = -ray.dir; BxDF::Type type;
		auto f = bsdf->sample(wo,glm::vec2(_dis(_gen),_dis(_gen)),wi,pdfFwd,type);

		if (f == glm::vec3(0.f) || pdfFwd == 0.f) break;
		beta *= f * std::abs(glm::dot(wi, vertex.normal)) / pdfFwd;
		pdfRev = bsdf->pdf(wo,wi);
		if (type & BxDF::SPECULAR) {
			point.delta = true;
			pdfRev = pdfFwd = 0;
		}
		//beta *= CorrectShadingNormal(isect, wo, wi, mode);
		ray = ray = Ray(vertex.pos,wi);

		// Compute reverse area density at preceding vertex
		//if(path.size() > 1)path[path.size()-2].pdfRev = vertex.ConvertDensity(pdfRev, prev);
	}
	return path;
}
