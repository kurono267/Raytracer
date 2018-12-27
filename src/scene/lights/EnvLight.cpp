//
// Created by kurono267 on 2018-12-26.
//

#include "EnvLight.hpp"

const glm::mat3 RGB_2_XYZ = glm::mat3(
		0.4124564, 0.3575761, 0.1804375,
		0.2126729, 0.7151522, 0.0721750,
		0.0193339, 0.1191920, 0.9503041
);

EnvLight::EnvLight(const spImage4f& image, const glm::vec3& light) : LightSource(Env,glm::mat4(1.0f)),_image(image),_light(light) {
	Image1f lightImage(image->size());
	for(int y = 0;y<image->height();++y){
		float sinTheta = std::sin((float) M_PI * (y + .5f) / (float)image->height());
		for(int x = 0;x<image->width();++x){
			glm::vec3 color = (*image)(x,y);
			glm::vec3 xyz = RGB_2_XYZ*color;
			lightImage(x,y) = sinTheta*xyz.y;
		}
	}
	_dist = std::make_unique<Distribution2D>(lightImage.data(),image->width(),image->height());
}

glm::vec3 EnvLight::sampleLi(const mango::sVertex &vertex, const glm::vec2 &sample, glm::vec3 &inWorld, float &pdf) {
	float mapPdf;
	glm::vec2 uv = _dist->sampleContinuous(sample, mapPdf);
	pdf = 0.0f;
	if (mapPdf == 0) return glm::vec3(0.f);

	float theta = uv.x * (float)M_PI, phi = uv.y * 2.f * (float)M_PI;
	float cosTheta = std::cos(theta), sinTheta = std::sin(theta);
	float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
	inWorld = _light2world*glm::vec4(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta,1.0f);

	pdf = mapPdf / (2 * (float)M_PI * (float)M_PI * sinTheta);
	if(!std::isfinite(pdf))pdf = 0.0f;

	return (*_image)(uv)*glm::vec4(_light,1.0f);
}

float EnvLight::pdfLi(const mango::sVertex &vertex, const glm::vec3 &inWorld) {
	return 1.0f;
}

const float worldRadius = 20.0f;

glm::vec3 EnvLight::power() {
	return (float)M_PI * worldRadius * worldRadius * (*_image)(glm::vec2(0.5f),_image->mipLevels()-1);
}
