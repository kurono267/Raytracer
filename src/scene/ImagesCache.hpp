//
// Created by kurono267 on 2018-12-21.
//

#ifndef RAYTRACER_IMAGESCACHE_HPP
#define RAYTRACER_IMAGESCACHE_HPP

#include <mango.hpp>
#include <unordered_map>

using namespace mango;

class ImagesCache {
public:
    spImage4b load(const std::string& filename);
private:
    std::unordered_map<std::string,spImage4b> _cache;
};

typedef std::shared_ptr<ImagesCache> spImagesCache;

#endif //RAYTRACER_IMAGESCACHE_HPP
