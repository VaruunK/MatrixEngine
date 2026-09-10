#include "TypeRegistry.hpp"
#include "Core/Structs/ReflectionStructs.hpp"
#include <stdexcept>
#include <typeindex>
#include <glm/ext/vector_float3.hpp>

bool TypeRegistry::RegisterType(const std::string& name, const Reflection* reflection) {
    auto& reg = registry();
    if (reg.contains(name)) {
        throw std::runtime_error("Type already exists: " + name);
    }
    reg[name] = reflection;
    return true;
}

const Reflection& TypeRegistry::Get(const std::string& name) {
    std::string valName = name;
    while(valName.ends_with("*") || valName.ends_with("&")) {
        valName = name.substr(0, name.size() - 1);
    }
    auto& reg = registry();
    auto it = reg.find(valName);
    if (it == reg.end()) {
        throw std::runtime_error("Type not registered: " + valName);
    }
    return *it->second;
}

std::unordered_map<std::string, const Reflection*> TypeRegistry::createregistry() {
    static std::unordered_map<std::string, const Reflection*> m;
    m.emplace("glm::vec3", new Primitive("glm::vec3", sizeof(glm::vec3)));
    m.emplace("std::string", new Primitive("std::string", sizeof(std::string)));
    m.emplace("std::type_index", new Primitive("std::type_index", sizeof(std::type_index)));

    m.emplace("int", new Primitive("int", sizeof(int)));
    m.emplace("float", new Primitive("float", sizeof(float)));
    m.emplace("double", new Primitive("double", sizeof(double)));
    m.emplace("long", new Primitive("long", sizeof(long)));
    m.emplace("short", new Primitive("short", sizeof(short)));
    m.emplace("char", new Primitive("char", sizeof(char)));
    m.emplace("bool", new Primitive("bool", sizeof(bool)));

    m.emplace("size_t", new Primitive("size_t", sizeof(size_t)));

    m.emplace("uint8_t", new Primitive("uint8_t", sizeof(uint8_t)));
    m.emplace("uint16_t", new Primitive("uint16_t", sizeof(uint16_t)));
    m.emplace("uint32_t", new Primitive("uint32_t", sizeof(uint32_t)));
    m.emplace("uint64_t", new Primitive("uint64_t", sizeof(uint64_t)));

    m.emplace("char8_t", new Primitive("char8_t", sizeof(char8_t)));
    m.emplace("char16_t", new Primitive("char16_t", sizeof(char16_t)));
    m.emplace("char32_t", new Primitive("char32_t", sizeof(char32_t)));
    m.emplace("wchar_t", new Primitive("wchar_t", sizeof(wchar_t)));

    return m;
}