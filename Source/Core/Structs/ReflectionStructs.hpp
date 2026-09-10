#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <cstring>
#include <cstdint>

enum class ReflectionType {
    CLASS,
    STRUCT,
    ENUM,
    PRIMITIVE,
    UNKNOWN
};

enum class ContainerType {
    SINGLE,
    MAP,

    // TODO need versions of these as well probably?

    ARRAY,
    VECTOR,
    SET,
    QUEUE,
    STACK,

    // TODO potential memory contianers later?
};

class AbstractTypeInfo {
public:
    AbstractTypeInfo(ContainerType containerType = ContainerType::SINGLE) : containerType(containerType) {}
    virtual ~AbstractTypeInfo() = default;
    std::string typeName = "None";
    bool isConst = false;
    size_t size = 0;
    ContainerType containerType;
};

class TypeInfo : public AbstractTypeInfo {
public:
    TypeInfo(ContainerType containerType = ContainerType::SINGLE)
        : AbstractTypeInfo(containerType) {}
};

class MapTypeInfo : public TypeInfo {
public:
    MapTypeInfo() : TypeInfo(ContainerType::MAP) {}
    std::shared_ptr<TypeInfo> keyInfo;
    std::shared_ptr<TypeInfo> valInfo;
    void (*forEachEntry)(void* mapObj, void (*visit)(const void* keyPtr, void* valPtr, void* userdata), void* userdata) = nullptr;
};

struct Field {
    std::string fieldName;
    std::shared_ptr<TypeInfo> typeInfo;
    std::map<std::string, std::string> specifiers;
    std::string value;
    size_t offset;
};

struct Function {
    std::string funcName;
    std::shared_ptr<TypeInfo> returnTypeInfo;
    std::map<std::string, std::string> specifiers;
    std::vector<std::pair<std::string, std::shared_ptr<TypeInfo>>> arguments;
    std::vector<std::pair<std::string, std::string>> defaultArgumentValues;
    void (*invoke)(void* obj, void** args, void* outReturn);
    bool isVirtual = false;
    bool isOverride = false;
    bool isConst = false;
    bool isFinal = false;
    bool isNoExcept = false;
    size_t line = 0;
};

class Reflection {
public:
    Reflection(std::string name, size_t size, ReflectionType type)
        : typeInfo(std::make_shared<TypeInfo>()), type(type) {
        typeInfo->typeName = std::move(name);
        typeInfo->size = size;
        typeInfo->isConst = false;
    }
    virtual ~Reflection() = default;
    std::shared_ptr<TypeInfo> typeInfo;
    ReflectionType type;
};

class Class : public Reflection {
public:
    Class(std::string name = "None", size_t size = 0) :
        Reflection(std::move(name), size, ReflectionType::CLASS) {};

    std::string parent = "None";
    void* (*createInstance)() = nullptr;
    std::map<std::string, std::vector<Field>> fields;
    std::map<std::string, std::vector<Function>> functions;
};

class Struct : public Reflection {
public:
    Struct(std::string name = "None", size_t size = 0) :
        Reflection(std::move(name), size, ReflectionType::STRUCT) {
    };
    std::map<std::string, std::string> specifiers;
    std::vector<std::pair<std::string, std::shared_ptr<TypeInfo>>> members;
    std::vector<std::pair<std::string, std::string>> defaultMemberValues;
    std::map<std::string, size_t> memberOffsets;
};

// primitives should be registered in type registry construction 

class Primitive : public Reflection {
public:
    Primitive(std::string name = "None", size_t size = 0) :
        Reflection(std::move(name), size, ReflectionType::PRIMITIVE) {};
};

inline void GetStructMemberValue(const Struct& structInfo, const std::string& memberName, const void* obj, void* out) {
    size_t offset = structInfo.memberOffsets.at(memberName);
    for (auto& member : structInfo.members) {
        if (member.first == memberName) {
            size_t size = member.second->size;
            const void* fieldPtr = static_cast<const uint8_t*>(obj) + offset;
            memcpy(out, fieldPtr, size);
            return;
        }
    }
}

inline void SetStructMemberValue(const Struct& structInfo, const std::string& memberName, void* obj, const void* in) {
    size_t offset = structInfo.memberOffsets.at(memberName);
    for (auto& member : structInfo.members) {
        if (member.first == memberName) {
            size_t size = member.second->size;
            void* fieldPtr = static_cast<uint8_t*>(obj) + offset;
            memcpy(fieldPtr, in, size);
            return;
        }
    }
}