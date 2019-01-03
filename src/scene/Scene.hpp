#pragma once

#include <boost/filesystem.hpp>

#include "Model.hpp"
#include <boost/property_tree/json_parser.hpp>
#include "lights/LightSource.hpp"

class Scene {
	const std::string binExt = "r267b";
	const std::string mtlExt = "json";
	public:
		Scene();
		virtual ~Scene();

		void add(const spModel& model);
		void add(const spLightSource& light);
		
		// Filename without resolution
		// Save and load 2 files 
		// filename.r267b -- meshes
		// filename.json -- materials
		void load(const std::string& filename);
		void save(const std::string& filename);

		void loadGLTF(const std::string& filename);

		std::string getFilename();

		std::vector<spModel>& models();
		std::unordered_map<std::string,spMaterial>& materials();
		std::vector<spLightSource>& lights();

		bool equal(const std::shared_ptr<Scene>& scene);
	protected:
		std::vector<spModel> _models;
		std::vector<spLightSource> _lights;
		std::unordered_map<std::string,spMaterial> _materials;
		std::string          _filename;

		spImagesCache _cache;
};

typedef std::shared_ptr<Scene> spScene;