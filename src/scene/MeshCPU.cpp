#include "MeshCPU.hpp"

using namespace mango;

MeshCPU::MeshCPU(const bool storeCPU) : _isData(false),_storeCPU(storeCPU) {}
MeshCPU::~MeshCPU(){}

void MeshCPU::setData(const uint8_t* data,const uint64_t& size){
	if(data == nullptr)throw std::runtime_error("Mesh::setData data is null");
	unpack(data,size);
	_isData = true;
}

void MeshCPU::setData(const std::vector<sVertex>& vertexes,const std::vector<uint32_t>& indexes,const std::string& name){
	_name     = name;
	_vertexes = vertexes;
	_indexes  = indexes;
	_isData   = true;
}

void MeshCPU::unpack(const uint8_t* data,const uint64_t& size){
	const size_t ui32 = sizeof(uint32_t);
	uint32_t offset = 0;
	// Unpack name of mesh
	uint32_t name_size;
	std::memcpy(&name_size,data,ui32);offset += ui32;
	
	char* name_str = new char[name_size];
	std::memcpy(name_str,data+offset,name_size); offset += name_size;
	_name = std::string(name_str);
	_name.resize(name_size);

	// Unpack Vertex buffer
	uint32_t format;
	std::memcpy(&format,data+offset,ui32); offset += ui32;
	if(format != (uint32_t)R267Format::R267VB)throw std::logic_error("Wrong format VB");

	uint32_t vb_size;
	std::memcpy(&vb_size,data+offset,ui32); offset += ui32;

	_vertexes.resize(vb_size);
	std::memcpy(_vertexes.data(),data+offset,vb_size*sizeof(sVertex)); offset += vb_size*sizeof(sVertex);

	std::memcpy(&format,data+offset,ui32); offset += ui32;
	if(format != (uint32_t)R267Format::R267IB)throw std::logic_error("Wrong format IB");

	uint32_t ib_size;
	std::memcpy(&ib_size,data+offset,ui32); offset += ui32;

	_indexes.resize(ib_size);
	std::memcpy(_indexes.data(),data+offset,ib_size*ui32); offset += ib_size*ui32;

	if(offset != size)throw std::logic_error("Checksum failed");
}

uint8_t* MeshCPU::pack(uint64_t& size){
	// Packed data
	size = 0; 

	const size_t ui32 = sizeof(uint32_t);

	// Compute Size of packed data
	size += ui32;                                 // Size of mesh name
	size += _name.size();                         // Mesh name
	size += ui32;                                 // VB ID
	size += ui32;                                 // VB SIZE
	size += sizeof(sVertex)*_vertexes.size();     // VERTEXES
	size += ui32;                                 // IB ID
	size += ui32;                                 // INDEX SIZE
	size += ui32*_indexes.size();                 // INDEXES

	// Allocation data
	uint8_t* data = new uint8_t[size];
	uint32_t offset = 0;
	uint32_t tmp = _name.size();
	std::memcpy(data,&tmp,ui32); offset += ui32; // Store size of mesh name

	std::memcpy(data+offset,_name.data(),_name.size()); offset += _name.size();

	tmp = (uint32_t)R267Format::R267VB;
	std::memcpy(data+offset,&tmp,ui32); offset += ui32;
	tmp = _vertexes.size();
	std::memcpy(data+offset,&tmp,ui32); offset += ui32;

	std::memcpy(data+offset,_vertexes.data(),_vertexes.size()*sizeof(sVertex)); offset += _vertexes.size()*sizeof(sVertex);
	tmp = (uint32_t)R267Format::R267IB;
	std::memcpy(data+offset,&tmp,ui32); offset += ui32;
	tmp = _indexes.size();
	std::memcpy(data+offset,&tmp,ui32); offset += ui32;

	std::memcpy(data+offset,_indexes.data(),_indexes.size()*ui32); offset += _indexes.size()*ui32;

	if(offset != size)throw std::logic_error("Checksum failed");
	return data;
}

uint8_t* MeshCPU::packed(uint64_t& size){
	return pack(size);
}

bool MeshCPU::is(){
	return _isData;
}

bool MeshCPU::equal(const std::shared_ptr<MeshCPU> other){
	if(!_isData || !other->_isData)return false;
	if(_vertexes.size() != other->_vertexes.size())return false;
	if(_indexes.size() != other->_indexes.size())return false;
	for(uint i = 0;i<_vertexes.size();++i){
		auto v0 = _vertexes[i];
		auto v1 = other->_vertexes[i];
		if(v0.pos != v1.pos)return false;
		if(v0.normal != v1.normal)return false;
		if(v0.uv != v1.uv)return false;
	}
	for(uint i = 0;i<_indexes.size();++i){
		auto i0 = _indexes[i];
		auto i1 = other->_indexes[i];
		if(i0 != i1)return false;
	}
	return true;
}

std::string MeshCPU::name(){
	return _name;
}
std::vector<sVertex>  MeshCPU::vertexes(){
	return _vertexes;
}
std::vector<uint32_t> MeshCPU::indexes(){
	return _indexes;
}

mango::spMesh createMesh(const mango::spDevice& device,const spMeshCPU& meshCPU){
	mango::spMesh mesh = std::make_shared<mango::Mesh>();
	mesh->create(device,meshCPU->vertexes(),meshCPU->indexes());
	return mesh;
}
