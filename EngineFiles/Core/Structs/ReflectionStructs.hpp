#pragma once

#include <vector>
#include <map>
#include <string>

struct ReflectedField {
    std::string name;
    std::string typeName;
    std::map<std::string, std::string> specifiers;
    std::string value;
    bool isConst = false;
    size_t offset;
    size_t size;
    size_t line;
};

struct ReflectedFunction {
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

struct ReflectedClass {
    std::string name;
    std::string parent = "None";
    size_t size;
    void* (*createInstance)();
    std::map<std::string, std::vector<ReflectedField>> fields;
    std::map<std::string, std::vector<ReflectedFunction>> functions;
};

struct ReflectedStruct {
    std::string name;
    std::map<std::string, std::string> specifiers;
    std::vector<std::pair<std::string, std::string>> members;
    std::vector<std::pair<std::string, std::string>> defaultMemberValues;
    std::map<std::string, size_t> memberOffsets;
    std::map<std::string, size_t> memberSizes;
    size_t size;
};

struct ReflectedStructInfo {
    std::string name;
    size_t size;
    std::vector<ReflectedField> members;
};

inline void GetStructMemberValue(const ReflectedStruct& structInfo, const std::string& memberName, const void* obj, void* out) {
    size_t offset = structInfo.memberOffsets.at(memberName);
    size_t size = structInfo.memberSizes.at(memberName);
    const void* fieldPtr = static_cast<const uint8_t*>(obj) + offset;
    memcpy(out, fieldPtr, size);
}

inline void SetStructMemberValue(const ReflectedStruct& structInfo, const std::string& memberName, void* obj, const void* in) {
    size_t offset = structInfo.memberOffsets.at(memberName);
    size_t size = structInfo.memberSizes.at(memberName);
    void* fieldPtr = static_cast<uint8_t*>(obj) + offset;
    memcpy(fieldPtr, in, size);
}