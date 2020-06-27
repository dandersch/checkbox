#pragma once
#include "pch.h"

template<typename Resource>
class ResourcePool
{
public:
    ResourcePool(const std::string& ext)
    {
        pool["missing"] = std::unique_ptr<Resource>(new Resource);
        pool["missing"]->loadFromFile("../res/missing" + ext);
    }

    Resource& get(const std::string& file)
    {
        if (pool.find(file) != pool.end())
        {
            // found
            return *(pool[file].get());
        }
        else
        {
            // not found
            pool[file] = std::unique_ptr<Resource>(new Resource);
            if (!pool[file]->loadFromFile("../res/" + file))
            {
                return *(pool["missing"].get());
            }
            else
            {
                return *(pool[file].get());
            }
        }
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Resource>> pool;
};
