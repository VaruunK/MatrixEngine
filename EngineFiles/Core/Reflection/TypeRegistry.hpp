#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <type_traits>

#include "Core/GameObject/GameObject.hpp"

struct FieldInfo {
    std::string name;
    std::string typeName;
    size_t offset;
};

struct ClassInfo {
    std::string name;
    std::string parentName;
    std::vector<FieldInfo> fields;
};

//class TypeRegistry {
//public:
//    template<typename GameObjectType> 
//    static void Register(const ClassInfo& info) {
//        static_assert(std::is_base_of_v<GameObject, GameObjectType>, "Type does not inherit from GameObject");
//        auto key = std::type_index(typeid(GameObjectType));
//        registry[key] = info;
//    }
//
//    template<typename GameObjectType>
//    static const ClassInfo* Get() {
//        auto key = std::type_index(typeid(GameObjectType));
//        auto it = registry.find(key);
//        return it != registry.end() ? &it->second : nullptr;
//    }
//
//    template<typename GameObjectType>
//    static std::vector<FieldInfo> GetAllFields() {
//        std::vector<FieldInfo> result;
//        auto key = std::type_index(typeid(GameObjectType));
//        const ClassInfo* current = Get<GameObjectType>();
//        while (current) {
//            result.insert(result.end(), current->fields.begin(), current->fields.end());
//            current = current->parentName.empty() ? nullptr : Get(current->parentName);
//        }
//        return result;
//    }
//
//private:
//    static inline std::unordered_map<std::type_index, ClassInfo> registry;
//};