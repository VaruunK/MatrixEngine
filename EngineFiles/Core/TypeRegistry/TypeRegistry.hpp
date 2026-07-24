#pragma once
#include <string>
#include <unordered_map>
#include "Core/MatrixAPI.hpp"

class Reflection;

class MATRIX_API TypeRegistry {
public:
    static bool RegisterType(const std::string& name, const Reflection* reflection);
    static const Reflection& Get(const std::string& name);
    static bool Contains(const std::string& name) {
        return registry().contains(name);
    }
private:
    static std::unordered_map<std::string, const Reflection*>& registry() {
        static std::unordered_map<std::string, const Reflection*> r;
        return r;
    }
};