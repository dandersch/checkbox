#pragma once

#include "pch.h"

template<typename Resource>
class ResourcePool
{
public:
    //ResourcePool(std::string& extension) {}
    ResourcePool()
    {
        pool["missing.png"] = std::unique_ptr<Resource>(new Resource);
        pool["missing.png"]->loadFromFile("../assets/missing.png");
    }

    ~ResourcePool() {}

    const Resource& get(const std::string& file)
    {

        if (pool.find(file) != pool.end()) {
            // found
            return *(pool[file].get());
        } else {
            // not found
            pool[file] = std::unique_ptr<Resource>(new Resource);
            if (!pool[file]->loadFromFile("../assets/" + file)) {
                return *(pool["missing.png"].get());
            } else {
                return *(pool[file].get());
            }
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Resource>> pool;
};
