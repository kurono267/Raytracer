#pragma once

#include <mango.hpp>
#include "Format.hpp"

class MeshCPU {
	public:
		MeshCPU(const bool storeCPU = false);
		virtual ~MeshCPU();

		void setData(const uint8_t* data,const uint64_t& size); // Set Packed data
		void setData(const std::vector<mango::sVertex>& vertexes,const std::vector<uint32_t>& indexes,const std::string& name); // Set Unpacked data

		std::string           name();
		std::vector<mango::sVertex>  vertexes();
		std::vector<uint32_t> indexes();

		bool equal(const std::shared_ptr<MeshCPU> other);

		uint8_t* packed(uint64_t& size);
		bool is(); 
	protected:
		void createShape();
		// Our data at CPU
		std::string          _name;
		std::vector<mango::sVertex> _vertexes;
		std::vector<uint32_t>    _indexes;

		bool _isData;
		bool _storeCPU;

		void unpack(const uint8_t* data,const uint64_t& size);
		uint8_t* pack(uint64_t& size);
};

typedef std::shared_ptr<MeshCPU> spMeshCPU;

mango::spMesh createMesh(const mango::spDevice& device,const spMeshCPU& meshCPU);
