//
// Created by XuriAjiva on 13.08.2023.
//

#pragma once

#include "defines.h"
#include "Core/Logger.h"

#include <vector>
#include <typeinfo>
#include <map>

inline std::map<const char*, std::vector<void*>> createdObjects;

// function to be called for every created object that needs to be destroyed
// stores the object and type in a vector
inline void AJ_RegisterCreated(void* object, const std::type_info& type)
{
    PLOG_VERBOSE << "Created: " << type.name() << " at: " << object;
    createdObjects[type.name()].push_back(object);
}

// function to be called for every destroyed object
// removes the object from the vector
inline void AJ_RegisterDestroyed(void* object, const std::type_info& type)
{
    PLOG_VERBOSE << "Destroyed: " << type.name() << " at: " << object;
    auto& vec = createdObjects[type.name()];
    auto it = std::find(vec.begin(), vec.end(), object);
    if (it != vec.end())
    {
        vec.erase(it);
    }
    else
    {
        PLOG_ERROR << "Object not found in createdObjects";
    }
}

// function to be called at the end of the program
// checks if all objects have been destroyed
inline void AJ_CheckForLeaks()
{
    bool leaks = false;
    for (auto& pair : createdObjects)
    {
        for (auto& object : pair.second)
        {
            PLOG_ERROR << "Leaked: " << pair.first << " at: " << object;
            leaks = true;
        }
    }
    if (leaks)
    {
        PLOG_FATAL << "Leaks detected!";
    }
    else
    {
        PLOG_INFO << "No leaks detected!";
    }
}
