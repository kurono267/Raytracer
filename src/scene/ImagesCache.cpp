//
// Created by kurono267 on 2018-12-21.
//

#include "ImagesCache.hpp"

spImage4b ImagesCache::load(const std::string &filename) {
    auto imageItr = _cache.find(filename);
    if(imageItr != _cache.end())
        return imageItr->second;

    spImage4b image = loadImage(filename);
    if(image != nullptr){
        _cache.emplace(filename,image);
    }
    return image;
}
