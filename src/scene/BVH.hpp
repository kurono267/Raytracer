#pragma once

#include "mango.hpp"
#include "Scene.hpp"
#include <algorithm>
#include "Ray.hpp"

#define NODE_MAX_TRIANGLE 1 // Max triangle in node

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
            int modelId;
			uint32_t mortonCode;
		};

		struct MeshID {
			int vbOffset;     int ibOffset;
			int numTriangles; int matID;
		};

		BVH();
		virtual ~BVH();

		void run(const spScene& scene);
		void runLBVH(const spScene& scene);

		RayHit intersect(const Ray& ray);
		sVertex postIntersect(const Ray &ray,const RayHit& hit);

		RayHit occluded(const Ray &ray);

		spScene getScene();

		std::vector<BVHNode>& nodes();
		size_t rootID();
	protected:
		void recursive(BVHNode& root, std::vector<Prim>& primitives,uint32_t start, uint32_t end, int depth, uint mesh_id); // BVH
		void intersect(const Ray& ray, RayHit& hit, BVHNode& node);

        void recursiveLBVH(BVHNode& root, const std::vector<Prim>& primitives,uint32_t start, uint32_t end, int depth, uint mesh_id);
        uint32_t findSplitLBVH(const std::vector<Prim>& primitives,uint32_t start, uint32_t end);

		std::vector<uint32_t> _indices;
		std::vector<sVertex>  _vertices;

		size_t rootId;
		std::vector<BVHNode> _nodes;
		std::vector<MeshID>  _meshIDs;
		std::vector<BVH>     _meshes;
		size_t			     _maxDepth;

		spScene _scene;
};
