#pragma once

#include <vector>
#include <map>
#include <string>

struct Field {
    std::string name;
    std::string typeName;
    std::map<std::string, std::string> specifiers;
    std::string value;
    bool isConst = false;
    size_t offset;
    size_t size;
    size_t line;
};

struct Function {
    std::string name;
    std::string returnType;
    std::map<std::string, std::string> specifiers;
    std::vector<std::pair<std::string, std::string>> arguments;
    std::vector<std::pair<std::string, std::string>> defaultArgumentValues;
    void (*invoke)(void* obj, void** args, void* outReturn);
    bool isVirtual = false;
    bool isOverride = false;
    bool isConst = false;
    bool isFinal = false;
    bool isNoExcept = false;
    size_t line;
};

enum class ReflectionType {
    CLASS, 
    STRUCT, 
    ENUM,
    UNKNOWN
};

class Reflection {
public:
    Reflection(std::string name, ReflectionType type) : name(std::move(name)), type(std::move(type)) {}
    virtual ~Reflection() = default;
    std::string name;
    ReflectionType type;
};

class Class : public Reflection {
public:
    Class(std::string name = "None") : Reflection(std::move(name), std::move(ReflectionType::CLASS)) {};
    std::string parent = "None";
    void* (*createInstance)() = nullptr;
    std::map<std::string, std::vector<Field>> fields;
    std::map<std::string, std::vector<Function>> functions;
    size_t size = 0;
};

class Struct : public Reflection {
public:
    Struct(std::string name = "None") : Reflection(std::move(name), std::move(ReflectionType::STRUCT)) {};
    std::map<std::string, std::string> specifiers;
    std::vector<std::pair<std::string, std::string>> members;
    std::vector<std::pair<std::string, std::string>> defaultMemberValues;
    std::map<std::string, size_t> memberOffsets;
    std::map<std::string, size_t> memberSizes;
    size_t size = 0;
};

inline void GetStructMemberValue(const Struct& structInfo, const std::string& memberName, const void* obj, void* out) {
    size_t offset = structInfo.memberOffsets.at(memberName);
    size_t size = structInfo.memberSizes.at(memberName);
    const void* fieldPtr = static_cast<const uint8_t*>(obj) + offset;
    memcpy(out, fieldPtr, size);
}

inline void SetStructMemberValue(const Struct& structInfo, const std::string& memberName, void* obj, const void* in) {
    size_t offset = structInfo.memberOffsets.at(memberName);
    size_t size = structInfo.memberSizes.at(memberName);
    void* fieldPtr = static_cast<uint8_t*>(obj) + offset;
    memcpy(fieldPtr, in, size);
}