#include "TypeRegistry.hpp"
#include "Core/Structs/ReflectionStructs.hpp"
#include <stdexcept>

bool TypeRegistry::RegisterType(const std::string& name, const Reflection* reflection) {
    auto& reg = registry();
    if (reg.contains(name)) {
        throw std::runtime_error("Type already exists: " + name);
    }
    reg[name] = reflection;
    return true;
}

const Reflection& TypeRegistry::Get(const std::string& name) {
    auto& reg = registry();
    auto it = reg.find(name);
    if (it == reg.end()) {
        throw std::runtime_error("Type not registered: " + name);
    }
    return *it->second;
}