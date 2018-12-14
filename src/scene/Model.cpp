#include "Model.hpp"

Model::Model(const std::string& name) : _name(name){}
Model::~Model(){}

spMaterial  Model::material(){
	return _material;
}
spMeshCPU      Model::mesh(){
	return _mesh;
}
std::string Model::name(){
	return _name;
}

void        Model::setMaterial(const spMaterial& material){
	_material = material;
}
void        Model::setMesh(const spMeshCPU& mesh){
	_mesh     = mesh;
}

bool Model::equal(const std::shared_ptr<Model>& model){
	if(!_mesh->equal(model->_mesh))return false;
	if(!_material->equal(model->_material))return false;
	return true;
}
