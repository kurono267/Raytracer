#pragma once

#include "mango.hpp"
#include "Scene.hpp"
#include <algorithm>
#include "Ray.hpp"

#define NODE_MAX_TRIANGLE 4 // Max triangle in node

std::ostream& operator<<(std::ostream& os,const glm::vec3& v);
std::ostream& operator<<(std::ostream& os, const BVHNode& n);

class BVH {
	public:
		struct Prim {
			Prim();

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

		RayHit intersect(const Ray& ray);
        RayHit intersectWithStack(const Ray &ray);
		sVertex postIntersect(const Ray &ray,const RayHit& hit);

		std::vector<BVHNode>& nodes();
		size_t rootID();
	protected:
		void recursive(BVHNode& root, std::vector<Prim>& primitives,uint32_t start, uint32_t end, int depth, uint mesh_id);
		void intersect(const Ray& ray, RayHit& hit, BVHNode& node);

		std::vector<uint32_t> _indices;
		std::vector<sVertex>  _vertices;

		size_t rootId;
		std::vector<BVHNode> _nodes;
		std::vector<MeshID>  _meshIDs;
		std::vector<BVH>     _meshes;
		size_t			     _maxDepth;
};
