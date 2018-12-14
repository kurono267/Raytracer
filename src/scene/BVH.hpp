#pragma once

#include "mango.hpp"
#include "Scene.hpp"
#include <algorithm> 

#define NODE_MAX_TRIANGLE 4 // Max triangle in node

namespace r267 {

struct BVHNode {
	BVHNode() : data(-1){}
	BVHNode(const BVHNode& n) : min(n.min),max(n.max),data(n.data) {}

	// Alligned BVH Node
    glm::vec4 min; // xyz min, w split
    glm::vec4 max; // xyz max, w 1 if leaf

    // Data changes by leaf or not current node
    // If Leaf x,y,z,w index of triangle
    // If Not leaf x,y r leaf, l leaf, z - meshID, w - depth
    glm::ivec4 data;
};

std::ostream& operator<<(std::ostream& os,const glm::vec3& v);
std::ostream& operator<<(std::ostream& os, const BVHNode& n);

class BVH {
	public:
		struct Prim {
			glm::vec3 minBox;
			glm::vec3 maxBox;
            glm::vec3 center;

			int id;
		};

		struct MeshID {
			int vbOffset;     int ibOffset;
			int numTriangles; int matID;
		};

		BVH();
		virtual ~BVH();

		void run(const spScene& scene);

		std::vector<BVHNode>& nodes();
		size_t rootID();
	protected:
		void recursive(BVHNode& root, std::vector<Prim>& primitives,const uint32_t start, const uint32_t end, const int depth, const uint& mesh_id);

		size_t rootId;
		std::vector<BVHNode> _nodes;
		std::vector<MeshID>  _meshIDs;
		std::vector<BVH>     _meshes;
		size_t			     _maxDepth;
};

}
