#include "BVH.hpp"
#include "BBox.hpp"

BVH::BVH() {}

BVH::~BVH(){}

std::ostream& operator<<(std::ostream& os, const BVHNode& n){
	os << "NODE" << std::endl;
	os << "MIN:" << glm::to_string(n.box.min) << std::endl;
	os << "MAX:" << glm::to_string(n.box.max) << std::endl;
	if(n.isLeaf == 1.0f)os << "DEPTH:" << n.data.depth << std::endl;
	return os;
}

inline float SurfaceArea(const BVHNode& n){
	glm::vec3 sides(n.box.max-n.box.min);
	return 2*(sides.x*sides.y+sides.y*sides.z+sides.x*sides.z);
}

BVH::Prim::Prim() {
    minBox = glm::vec3(std::numeric_limits<float>::infinity());
    maxBox = glm::vec3(-std::numeric_limits<float>::infinity());
    center = glm::vec3(0.f);
}

void BVH::run(const spScene& scene){
    std::vector<Prim> scene_prims;
    // All meshes in scene to one
    for(const auto& model : scene->models()){
        auto mesh = model->mesh();
        auto vbOffset = _vertices.size();
        auto ibOffset = _indices.size();
        std::copy(mesh->vertexes().begin(),mesh->vertexes().end(),back_inserter(_vertices));
        const auto& indices = mesh->indexes();
        for(int i = 0;i<indices.size();i+=3){
            Prim prim;
            for(int v = 0;v<3;++v){
                auto index = indices[i+v];
                _indices.emplace_back(static_cast<uint32_t>(vbOffset + index));
                auto vPos = mesh->vertexes()[index].pos;
                prim.minBox = glm::min(prim.minBox,vPos);
                prim.maxBox = glm::max(prim.maxBox,vPos);
                prim.center += vPos;
            }
            prim.id = static_cast<int>(ibOffset + i);
            prim.center /= 3.0f;
            scene_prims.push_back(prim);
        }
    }

    BVHNode root;
    _nodes.emplace_back();
    rootId = _nodes.size()-1;
    _maxDepth = 0;

    recursive(root,scene_prims,0,scene_prims.size(),0,0);

    _nodes[rootId] = root;
}

void BVH::recursive(BVHNode& root, std::vector<Prim>& primitives, uint32_t start, uint32_t end, int depth, uint mesh_id){
    if(_maxDepth < depth)_maxDepth = depth;
    BBox aabb;
    BBox centroid;
    // Recompute BBox
    for(int i = start;i<end;++i){
        const Prim& p = primitives[i];
        aabb.expand(p.minBox,p.maxBox);
        centroid.expand(p.center);
    }
    root.box = aabb;

    // Check leaf
    uint32_t size = end-start;
    if(size <= NODE_MAX_TRIANGLE){
        root.isLeaf = true;
        for(uint32_t i = 0;i<NODE_MAX_TRIANGLE;++i){
            if(i < size){
                root.triIds[i] = primitives[start+i].id;
            }
            else root.triIds[i] = -1;
        }
        return;
    }

    // Split axis, max axis in AABB
    uint32_t axis  = aabb.maxDim();

    float    split = 0.5f*(aabb.max[axis]-aabb.min[axis]);

    // Partly sort
    uint32_t  mid = start;
    for(uint32_t i = start;i<end;++i){
        if(primitives[i].center[axis] < split){
            std::iter_swap(primitives.begin()+i,primitives.begin()+mid);
            ++mid;
        }
    }

    if(mid == start || mid == end){
        mid = start + (end-start)/2;
    }

    // Right node
    BVHNode right;
    recursive(right, primitives, mid, end, depth + 1, mesh_id);
    _nodes.push_back(right);
    root.data.right = _nodes.size() - 1;

    BVHNode left;
    recursive(left,primitives,start,mid,depth+1,mesh_id);
    _nodes.push_back(left);
    root.data.left = _nodes.size()-1;

    // Fill additional data to node
    root.data.depth = depth;
    root.split  = split;
}

inline bool check(const glm::vec3& pos,const glm::vec3& min,const glm::vec3 max){
	if(pos.x < min.x || pos.y < min.y || pos.z < min.z)return false;
	if(pos.x > max.x || pos.y > max.y || pos.z > max.z)return false;
	return true;
}

std::vector<BVHNode>& BVH::nodes(){
	return _nodes;
}

size_t BVH::rootID(){
	return rootId;
}

const int MAX_STACK = 64;
struct Stack {
    int nodes[MAX_STACK];
    int top;
} stack;

void add_stack(int id){
    if(stack.top < MAX_STACK-1){
        stack.nodes[++stack.top] = id;
    }
}

using namespace glm;

RayHit BVH::intersectWithStack(const Ray &ray) {
    // Check root, if not return
    RayHit hit = intersectBBox(ray,_nodes[rootId].box);
    if(!hit.status)return hit;

    hit.status = false;
    hit.dist   = std::numeric_limits<float>::infinity();

    stack.nodes[0] = rootId;
    stack.top = 0;

    RayHit test;BVHNode curr;
    int currId;
    // Max steps for debug
    uint step = 0;
    while(stack.top >= 0){
        step++;
        // Get Top element from stack
        currId = stack.nodes[stack.top--];
        curr  = _nodes[currId];

        if(curr.isLeaf){ // Leaf
            // Check triangles
            test = intersectBBox(ray,curr.box);
            if(test.dist < hit.dist && test.dist > 0.0f){
                auto triIds = curr.triIds;
                for(int i = 0;i<4;++i){
                    int triId = triIds[i];
                    if(triId >= 0){
                        auto id0 = _indices[triId]; auto id1 = _indices[triId+1]; auto id2 = _indices[triId+2];
                        auto vp0 = _vertices[id0]; auto vp1 = _vertices[id1]; auto vp2 = _vertices[id2];
                        test = intersectTriangle(ray,vp0.pos,vp1.pos,vp2.pos);
                        if(test.dist < hit.dist && test.dist > 0.0f){
                            hit = test;
                            hit.id1 = triId;
                            //break;
                        }
                    }
                }
            }
        } else {
            // Check left and right child
            int leftID  = curr.data.left;
            int rightID = curr.data.right;

            RayHit left = intersectBBox(ray,_nodes[leftID].box);
            RayHit right = intersectBBox(ray,_nodes[rightID].box);

            bool leftStatus  = left.dist < hit.dist && left.dist > 0.0f;
            bool rightStatus = right.dist < hit.dist && right.dist > 0.0f;

            if(leftStatus && rightStatus){
                // Left child always neares, if not swap
                if(right.dist < left.dist){
                    int tmp = leftID;
                    leftID  = rightID;
                    rightID = tmp;
                }
                add_stack(rightID);
                add_stack(leftID); // Left ID at top
            } else if(leftStatus)add_stack(leftID);
            else if(rightStatus)add_stack(rightID);
        }

    }
    return hit;
}

RayHit BVH::intersect(const Ray &ray) {
    // Check root, if not return
    auto rootNode = _nodes[rootId];
    RayHit hit = intersectBBox(ray,_nodes[rootId].box);
    if(!hit.status)return hit;

    hit.status = false;
    hit.dist   = std::numeric_limits<float>::infinity();

    intersect(ray,hit,rootNode);

    return hit;
}

void BVH::intersect(const Ray &ray, RayHit& hit, BVHNode &curr) {
    if(curr.isLeaf){ // Leaf
        // Check triangles
        auto test = intersectBBox(ray,curr.box);
        if(test.dist < hit.dist && test.dist > 0.0f){
            auto triIds = curr.triIds;
            for(int i = 0;i<4;++i){
                int triId = triIds[i];
                if(triId >= 0){
                    auto id0 = _indices[triId]; auto id1 = _indices[triId+1]; auto id2 = _indices[triId+2];
                    auto vp0 = _vertices[id0]; auto vp1 = _vertices[id1]; auto vp2 = _vertices[id2];
                    test = intersectTriangle(ray,vp0.pos,vp1.pos,vp2.pos);
                    if(test.dist < hit.dist && test.dist > 0.0f){
                        hit = test;
                        hit.id1 = triId;
                        //break;
                    }
                }
            }
        }
    } else {
        // Check left and right child
        int leftID  = curr.data.left;
        int rightID = curr.data.right;

        auto leftNode = _nodes[leftID];
        auto rightNode = _nodes[rightID];

        RayHit left = intersectBBox(ray,_nodes[leftID].box);
        RayHit right = intersectBBox(ray,_nodes[rightID].box);

        bool leftStatus  = left.dist < hit.dist && left.dist > 0.0f;
        bool rightStatus = right.dist < hit.dist && right.dist > 0.0f;

        if(leftStatus && rightStatus){
            // Left child always nearest, if not swap
            if(right.dist < left.dist){
                std::swap(leftNode,rightNode);
            }
            intersect(ray,hit,leftNode);
            intersect(ray,hit,rightNode);
        } else if(leftStatus)intersect(ray,hit,leftNode);
        else if(rightStatus)intersect(ray,hit,rightNode);
    }
}

sVertex BVH::postIntersect(const Ray &ray,const RayHit &hit) {
    sVertex hitVertex;
    hitVertex.pos = ray.org+ray.dir*hit.dist;
    for(int v = 0;v<3;++v){
        const auto& id = _indices[hit.id1+v];
        const auto& vert = _vertices[id];
        hitVertex.normal += vert.normal*hit.bc[v];
        hitVertex.uv     += vert.uv*hit.bc[v];
    }
    return hitVertex;
}
