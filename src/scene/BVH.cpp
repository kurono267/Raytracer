#include "BVH.hpp"

BVH::BVH() {}

BVH::~BVH(){}

std::ostream& operator<<(std::ostream& os,const glm::vec3& v){
	os << v.x << "," << v.y << "," << v.z;
	return os;
}

std::ostream& operator<<(std::ostream& os, const BVHNode& n){
	os << "NODE" << std::endl;
    glm::vec3 tmp = glm::vec3(n.min);
	os << "MIN:" << tmp << std::endl;
    tmp = glm::vec3(n.max);
	os << "MAX:" << tmp << std::endl;
	if(n.max.w == 1.0f)os << "DEPTH:" << n.data.w << std::endl;
	return os;
}

inline float SurfaceArea(const BVHNode& n){
	glm::vec3 sides(n.max-n.min);
	return 2*(sides.x*sides.y+sides.y*sides.z+sides.x*sides.z);
}

void BVH::run(const spScene& scene){
    // Create BVHs for meshes in scene
    _meshes.resize(scene->models().size());
    _meshIDs.resize(_meshes.size());
    std::vector<Prim> scene_prims(_meshes.size());
    uint32_t ibOffset = 0;
    for(uint32_t i = 0;i<_meshes.size();++i){
        spModel model = scene->models()[i];
        spMeshCPU  mesh = model->mesh();

        auto vertexes  = mesh->vertexes();
        auto indexes = mesh->indexes();

        uint32_t numTriangles = indexes.size()/3;

        for(int i = 0;i<numTriangles;++i){
            int startTriangle = i*3;
            uint32_t v0 = indexes[startTriangle];

            Prim prim;

            glm::vec3 v0pos = vertexes[v0].pos;
            prim.minBox = v0pos;
            prim.maxBox = v0pos;
            prim.center = v0pos;
            prim.id = startTriangle+ibOffset;
            for(int v = 1;v<3;++v){
                glm::vec3 pos = vertexes[indexes[startTriangle+v]].pos;
                prim.minBox = min(prim.minBox,pos);
                prim.maxBox = max(prim.maxBox,pos);
                prim.center += pos;
            }
            prim.center /= 3.0f;
            scene_prims.push_back(prim);
        }


        ibOffset += mesh->indexes().size();
    }

    BVHNode root;
    root.data = glm::ivec4(0);
    _nodes.push_back(root);
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
    root.min = glm::vec4(aabb.min,0.0f);
    root.max = glm::vec4(aabb.max,0.0f);
    root.data = glm::ivec4(-1);

    // Check leaf
    uint32_t size = end-start;
    if(size <= NODE_MAX_TRIANGLE){
        root.max.w = 1.0f;
        for(uint32_t i = 0;i<NODE_MAX_TRIANGLE;++i){
            if(i < size){
                root.data[i] = primitives[start+i].id;
            }
            else root.data[i] = -1;
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
    root.data.y = _nodes.size() - 1;

    BVHNode left;
    recursive(left,primitives,start,mid,depth+1,mesh_id);
    _nodes.push_back(left);
    root.data.x = _nodes.size()-1;

    // Fill additional data to node
    root.data.z = axis; // Axis
    root.data.w = depth;
    root.min.w  = split;
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

const int MAX_STACK = 10;
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

RayHit BVH::intersect(const Ray &ray) {
    // Check root, if not return
    RayHit hit = intersectBBox(ray,_nodes[rootId]);
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

        if(curr.max.w == 1.0f){ // Leaf
            // Check triangles
            test = intersectBBox(ray,curr);
            if(test.dist < hit.dist){
                hit = test;
                //break;
                /*ivec4 triIds = curr.data;
                for(int i = 0;i<4;++i){
                    int triId = triIds[i];
                    if(triId >= 0){
                        Vertex vp0 = convert(vertexes[indexes[triId+0]]);Vertex vp1 = convert(vertexes[indexes[triId+1]]);Vertex vp2 = convert(vertexes[indexes[triId+2]]);
                        test = intersect(ray,vp0.pos,vp1.pos,vp2.pos);
                        if(test.dist < hit.dist){
                            hit = test;
                            hit.id1 = triId;
                            //break;
                        }
                    }
                }*/
            }
        } else {
            // Check left and right child
            int leftID  = curr.data.x;
            int rightID = curr.data.y;

            RayHit left = intersectBBox(ray,_nodes[leftID]);
            RayHit right = intersectBBox(ray,_nodes[rightID]);

            bool leftStatus  = left.dist < hit.dist;
            bool rightStatus = right.dist < hit.dist;

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

