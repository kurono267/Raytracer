#pragma once

#include <mango.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/optional/optional.hpp>

#include <unordered_map>

using namespace boost::property_tree;
using namespace mango;

struct MaterialUBO {
	// Material data
	glm::vec4   diffuseColor;  // Diffuse Color
	glm::vec4   data;          // X type of Shading, Y roughness, Z metallic, W mesh ID
};

class Material {
	public:
		enum SurfaceType {
			DefaultSurface    = 0,
			NormalSurface  = 1,
			ParallaxSurface   = 2
		};

		Material();
		virtual ~Material();

		void setPath(const std::string& path);
		
		void read(const ptree& root,const std::string& object); // Read json material from filename

		void save(ptree& root,const std::string& object);

		void setMetallic(const float& metallic);
		void setRoughness(const float& roughness);
		void setDiffuseColor(const glm::vec3& color);

		float getMetallic();
		float getRoughness();
		glm::vec3 getDiffuseColor();

		void setDiffuseTexture(const std::string& filename);
		void setNormalTexture(const std::string& filename);
		void setHeightmapTexture(const std::string& filename);

		bool equal(const std::shared_ptr<Material>& material);

		//void create(spDevice device,std::unordered_map<std::string,spImage>& imagesBuffer);

		MaterialUBO data();

		spDescSet   getDescSet();
	protected:
		void read(const ptree& tree);           // Read material from ptree
		void save(ptree& tree);
		// Material data
		MaterialUBO _data;
		// Material Texture
		std::string _diffuseFilename;
		std::string _normalFilename;
		std::string _heightmapFilename;

		Uniform       _uniform;
		spTexture     _diffTexture;
		spTexture     _normalTexture;
		spTexture     _heightmapTexture;
		spDescSet     _descSet;

		std::string   _path;
};

typedef std::shared_ptr<Material> spMaterial;