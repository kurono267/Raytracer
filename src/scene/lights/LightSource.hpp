//
// Created by kurono267 on 2018-12-26.
//

#pragma once

#include <mango.hpp>

class LightSource {
	public:
		enum Type {
			Point,
			Dir,
			Area,
			Env
		};
	public:
		LightSource(const Type _type, const glm::mat4& light2world)
		: type(_type),_light2world(light2world),_world2light(glm::inverse(light2world)) {}
		virtual ~LightSource() = default;

	 	bool isDelta() { return type == Point || type == Dir; }

		virtual glm::vec3 sampleLi(const mango::sVertex& vertex, const glm::vec2& sample, glm::vec3& inWorld, float& pdf) = 0;
		virtual float pdfLi(const mango::sVertex& vertex, const glm::vec3& inWorld) = 0;

		virtual glm::vec3 power() = 0;
	protected:
		const uint32_t type;
		glm::mat4 _light2world;
		glm::mat4 _world2light;
};

typedef std::shared_ptr<LightSource> spLightSource;

