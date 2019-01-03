#include "Scene.hpp"
#include "Format.hpp"

namespace fs = boost::filesystem;

Scene::Scene() : _cache(std::make_shared<ImagesCache>()){}
Scene::~Scene(){}

void Scene::add(const spModel& model){
	_models.push_back(model);
}

void Scene::load(const std::string& filename){
	// Read binary meshes
	// Check filesize
	_filename = filename;
	std::string binFilename = filename + "." + binExt;
	std::cout << binFilename << std::endl;
	fs::path path = fs::canonical(binFilename);

    uint64_t size = fs::file_size(path);
    // Create main file buffer
    uint8_t* data = new uint8_t[size];

    const size_t ui32 = sizeof(uint32_t);

    std::ifstream file(binFilename,std::ios::in | std::ios::binary);
    file.read((char*)data,size);
    file.close();

    uint32_t numMeshes = 0;

    size_t offset = 0;
    uint32_t tmp;
	std::memcpy(&tmp,data+offset,ui32); offset += ui32;
	if(tmp != (uint32_t)R267Format::R267Magic)throw std::logic_error("Wrong magic");
	std::memcpy(&numMeshes,data+offset,ui32); offset += ui32;

	for(uint32_t m = 0;m<numMeshes;++m){
		std::memcpy(&tmp,data+offset,ui32); offset += ui32;
		if(tmp != (uint32_t)R267Format::R267Mesh)throw std::logic_error("Wrong mesh");

		uint64_t pack_size;
		std::memcpy(&pack_size,data+offset,sizeof(uint64_t)); offset += sizeof(uint64_t);
		
		spMeshCPU mesh = std::make_shared<MeshCPU>();
		mesh->setData(data+offset,pack_size);
		offset += pack_size;

		spModel model = std::make_shared<Model>(mesh->name());
		model->setMesh(mesh);
		_models.push_back(model);
	}
	if(offset != size){
		throw std::logic_error("Checksum failed");
	}
	delete[] data;

	ptree root;
	json_parser::read_json(filename + "." + mtlExt,root);
	// Read material
	for(auto m : _models){
		spMaterial material;
		if(_materials.find(m->name()) == _materials.end()){
			material = std::make_shared<Material>(_cache);
			auto matPath = fs::path(filename).remove_filename();
			material->setPath(matPath.string());
			material->read(root,m->name());
			_materials.insert(std::pair<std::string,spMaterial>(m->name(),material));
		} else material = _materials[m->name()];
		m->setMaterial(material);
	}
}

void Scene::save(const std::string& filename){
	// Save meshes
	// Packed meshes
	std::vector<std::pair<uint64_t,uint8_t*> > packed_models;
	uint64_t size = 0;
	const size_t ui32 = sizeof(uint32_t);
	// Format description size
	size += 2*ui32;
	for(auto m : _models){
		size += ui32;
		size += sizeof(uint64_t);
		std::pair<uint64_t,uint8_t*> pack;
		pack.second = m->mesh()->packed(pack.first);
		packed_models.push_back(pack);
		size += pack.first;
	}
	// Create file data
	uint8_t* data = new uint8_t[size];
	size_t offset = 0;
	uint32_t tmp = (uint32_t)R267Format::R267Magic;
	std::memcpy(data+offset,&tmp,ui32); offset += ui32;
	tmp = packed_models.size();
	std::memcpy(data+offset,&tmp,ui32); offset += ui32;

	for(auto &pack : packed_models){
		tmp = (uint32_t)R267Format::R267Mesh;

		std::memcpy(data+offset,&tmp,ui32); offset += ui32;
		std::memcpy(data+offset,&pack.first,sizeof(uint64_t)); offset += sizeof(uint64_t);
		std::memcpy(data+offset,pack.second,pack.first); offset += pack.first;
		delete[] pack.second;
	}

	if(offset != size){
		throw std::logic_error("Checksum failed");
	}

	std::ofstream file (filename + "." + binExt, std::ios::out | std::ios::binary);
    file.write ((const char*)data, size);
    file.close();

    delete[] data;

    // Save materials
    ptree root;
    for(auto m : _materials){
    	m.second->save(root,m.first);
    }
    json_parser::write_json(filename + "." + mtlExt,root);
}

std::string Scene::getFilename(){
	return _filename;
}

std::vector<spModel>& Scene::models(){
	return _models;
}

std::unordered_map<std::string,spMaterial>& Scene::materials(){
	return _materials;
}

bool Scene::equal(const std::shared_ptr<Scene>& scene){
	if(_models.size() != scene->_models.size())return false;
	for(int i = 0;i<_models.size();++i){
		auto m = _models[i];
		if(!m->equal(scene->_models[i]))return false;
	}
	return true;
}

void Scene::add(const spLightSource &light) {
	_lights.push_back(light);
}

std::vector<spLightSource> &Scene::lights() {
	return _lights;
}

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

void Scene::loadGLTF(const std::string &filename) {
	tinygltf::Model tfModel;
	tinygltf::TinyGLTF tfLoader;
	std::string err;
	std::string warn;

	bool ret = tfLoader.LoadASCIIFromFile(&tfModel, &err, &warn, filename);
	if(!ret){
		std::cout << "LoadGLTF " << filename << " Failed " << err << std::endl;
		return;
	}
	std::cout << "LoadGLTF " << filename << std::endl;
	if(!warn.empty())std::cout << "Warnings! " << warn << std::endl;

	for(auto tfMesh : tfModel.meshes){
		for(auto tfPrim : tfMesh.primitives){
			spMeshCPU mesh = std::make_shared<MeshCPU>();
			std::vector<sVertex> vertices;

			const tinygltf::Accessor& posAccess = tfModel.accessors[tfPrim.attributes["POSITION"]];
			const tinygltf::BufferView& posBufferView = tfModel.bufferViews[posAccess.bufferView];
			vertices.resize(posAccess.count);
			const tinygltf::Buffer& posBuffer = tfModel.buffers[posBufferView.buffer];
			const float* positions = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccess.byteOffset]);
			for (size_t i = 0; i < posAccess.count; ++i) {
				vertices[i].pos = glm::vec3(positions[i * 3 + 0],positions[i * 3 + 1],positions[i * 3 + 2]);
			}

			const tinygltf::Accessor& normalAccess = tfModel.accessors[tfPrim.attributes["NORMAL"]];
			const tinygltf::BufferView& normalBufferView = tfModel.bufferViews[normalAccess.bufferView];
			if(normalAccess.count){
				const tinygltf::Buffer& normalBuffer = tfModel.buffers[posBufferView.buffer];
				const float* normals = reinterpret_cast<const float*>(&normalBuffer.data[normalBufferView.byteOffset + normalAccess.byteOffset]);
				for (size_t i = 0; i < posAccess.count; ++i) {
					vertices[i].normal = glm::vec3(normals[i * 3 + 0],normals[i * 3 + 1],normals[i * 3 + 2]);
				}
			}

			const tinygltf::Accessor& uvAccess = tfModel.accessors[tfPrim.attributes["TEXCOORD_0"]];
			const tinygltf::BufferView& uvBufferView = tfModel.bufferViews[uvAccess.bufferView];
			if(uvAccess.count){
				const tinygltf::Buffer& uvBuffer = tfModel.buffers[uvBufferView.buffer];
				const float* uvs = reinterpret_cast<const float*>(&uvBuffer.data[uvBufferView.byteOffset + uvAccess.byteOffset]);
				for (size_t i = 0; i < uvAccess.count; ++i) {
					vertices[i].uv = glm::vec2(uvs[i * 2 + 0],uvs[i * 2 + 1]);
				}
			}

			std::vector<uint32_t> indices;
			const tinygltf::Accessor& indicesAccess = tfModel.accessors[tfPrim.indices];
			const tinygltf::BufferView& indicesBufferView = tfModel.bufferViews[indicesAccess.bufferView];
			indices.resize(indicesAccess.count);
			const tinygltf::Buffer& indicesBuffer = tfModel.buffers[indicesBufferView.buffer];
			const uint32_t* tfIndices = reinterpret_cast<const uint32_t*>(&indicesBuffer.data[indicesBufferView.byteOffset + indicesAccess.byteOffset]);
			for(int i = 0;i<indicesAccess.count;++i){
				indices[i] = tfIndices[i];
			}
			mesh->setData(vertices,indices,tfMesh.name);

			spMaterial mat = std::make_shared<Material>(_cache);

			auto matPath = fs::path(filename).remove_filename();
			mat->setPath(matPath.string());

			auto tfMaterial = tfModel.materials[tfPrim.material];
			auto baseColorFactorItr = tfMaterial.values.find("baseColorFactor");
			if(baseColorFactorItr != tfMaterial.values.end()){
				auto colorFactor = baseColorFactorItr->second.ColorFactor();
				mat->setDiffuseColor(glm::vec3(colorFactor[0],colorFactor[1],colorFactor[2]));
			}
			auto baseColorTextureItr = tfMaterial.values.find("baseColorTexture");
			if(baseColorTextureItr != tfMaterial.values.end()){
				auto textureId = baseColorTextureItr->second.TextureIndex();
				mat->setDiffuseTexture(tfModel.images[textureId].uri);
			}
			auto roughnessFactorItr = tfMaterial.values.find("roughnessFactor");
			if(roughnessFactorItr != tfMaterial.values.end()) {
				mat->setRoughness(roughnessFactorItr->second.Factor());
			}

			spModel model = std::make_shared<Model>(tfMesh.name);
			model->setMesh(mesh);
			model->setMaterial(mat);

			add(model);
		}
	}

	std::cout << "Finish Loading " << std::endl;
}
