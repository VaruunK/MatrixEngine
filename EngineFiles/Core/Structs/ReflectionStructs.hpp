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