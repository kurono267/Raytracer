#pragma once

#include "Material.hpp"
#include "MeshCPU.hpp"

using namespace mango;

// Our Representation of Model
class Model {
	public:
		Model(const std::string& name);
		virtual ~Model();
		
		spMaterial  material();
		spMeshCPU      mesh();
		std::string name();

		void        setMaterial(const spMaterial& material);
		void        setMesh(const spMeshCPU& mesh);

		bool equal(const std::shared_ptr<Model>& model);
	protected:
		std::string _name;
		spMaterial  _material;
		spMeshCPU      _mesh;
};

typedef std::shared_ptr<Model> spModel;
