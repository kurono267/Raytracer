//
// Created by kurono267 on 2018-12-14.
//

#include "PathTracer.hpp"

PathTracer::PathTracer(const spDevice &device, const spScene &scene, int width, int height) : Integrator(device, scene,
																										 width,
																										 height) {}

void PathTracer::init() {

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
	glm::vec2 ptSize(_width,_height);

	for(int yBlock = start.y;yBlock<start.y+tileSize;yBlock+=2){
		if(yBlock >= _height)break;
		for(int xBlock = start.x;xBlock<start.x+tileSize;xBlock+=2){
			if(xBlock >= _width)break;

			// Intersection Block
			for(int yB = 0;yB<2;++yB) {
				int y = yBlock+yB;
				if(y >= _height)break;
				for(int xB = 0;xB<2;++xB) {
					int x = xBlock+xB;
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
				if(y >= _height)break;
				for(int xB = 0;xB<2;++xB) {
					int x = xBlock + xB;
					if (x >= _width)break;

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
