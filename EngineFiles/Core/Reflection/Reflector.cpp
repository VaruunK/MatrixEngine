#include <filesystem>
#include <fstream>
#include <array>
#include <iostream>
#include "../../Core/Structs/ReflectionStructs.hpp"

enum class TokenKind {
    Identifier, 
    Assignment, 
    Punctuation, 
    Semicolon, 
    Comma, 
    OpenParen, 
    CloseParen, 
    OpenAngle,
    CloseAngle,
    StringLiteral,
    NumberLiteral,
    EndOfFile
};

enum class FieldState { Scanning, ParsingArgs, ParsingDeclType, ParsingDeclName, Done };
enum class FunctionState { Scanning, ParsingArgs, ParsingRetType, ParsingFuncArgs, Done };
enum class StructState { Scanning, ParsingArgs, ParsingStructName, ParsingMemberType, ParsingMemberName, Done };

struct ParsedItems {
    std::vector<ReflectedClass> classes;
    std::vector<ReflectedStruct> structs;
};

struct Token {
    TokenKind kind;
    std::string text;
    size_t line;
};

struct TokenStream {
    const std::vector<Token>& tokens;
    size_t pos = 0;

    const Token& peek(size_t ahead = 0) const {
        size_t idx = pos + ahead;
        return idx < tokens.size() ? tokens[idx] : tokens.back();
    }
    const Token& advance() {
        if (pos < tokens.size()) {
            return tokens[pos++];
        }
        return tokens.back();
    }
    
    const Token& skip(size_t ahead) { 
        for (size_t i = 0; i < ahead; i++) {
            if (atEnd()) {
                break;
            }
            advance();
        }
        return peek();
    }
    
    bool atEnd() const { return peek().kind == TokenKind::EndOfFile; }
};

static bool isTightToken(const std::string& t) {
    return t == "::" || t == "<" || t == ">" || t == "*" || t == "&" || t == "(" || t == ")";
}

static std::string joinTokens(const std::vector<std::string>& toks) {
    std::string result;
    for (size_t i = 0; i < toks.size(); i++) {
        if (i > 0 && !toks[i - 1].empty() && !toks[i].empty()
            && !isTightToken(toks[i - 1]) && !isTightToken(toks[i])) {
            result += " ";
        }
        result += toks[i];
    }
    return result;
}

static bool tryFinishFunction(TokenStream& ts, ReflectedFunction& current) {
    // Skip known trailing qualifiers: const, override, final, noexcept
    while (ts.peek().kind == TokenKind::Identifier) {
        const std::string& kw = ts.peek().text;
        if (kw == "const") current.isConst = true;
        if (kw == "override") current.isOverride = true;
        if (kw == "final") current.isFinal = true;
        if (kw == "noexcept") current.isNoExcept = true;
        if (kw == "const" || kw == "override" || kw == "final" || kw == "noexcept") {
            ts.advance();
            continue;
        }
        break;
    }

    if (ts.peek().kind == TokenKind::Semicolon) {
        ts.advance();
        return true;
    }

    if (ts.peek().text == "{") {
        // Consume a full balanced-brace body so parseClass never sees
        // an unmatched '}' from an inline function definition.
        int depth = 0;
        do {
            if (ts.peek().text == "{") depth++;
            if (ts.peek().text == "}") depth--;
            ts.advance();
        } while (depth > 0 && !ts.atEnd());
        return true;
    }

    return false;
}

static ReflectedFunction parseFunction(TokenStream& ts) {
    FunctionState state = FunctionState::Scanning;
    
    ReflectedFunction current;
    std::vector<std::string> returnTypeTokens;
    std::vector<std::string> argTypeTokens;

    int parenDepth = 0;
    int angleDepth = 0;
    bool sawArgListOpenParen = false;

    while (!ts.atEnd()) {
        const Token& tok = ts.peek();

        switch (state) {

        case FunctionState::Scanning:
            if (tok.kind == TokenKind::Identifier) {
                current = ReflectedFunction{};
                current.line = tok.line;
                returnTypeTokens.clear();

                bool nextIsParen = ((ts.peek(1).kind != TokenKind::EndOfFile) && (ts.peek(1).kind == TokenKind::OpenParen));

                if (nextIsParen) {
                    state = FunctionState::ParsingArgs;
                }
            }
            break;

        case FunctionState::ParsingArgs: {
            if (tok.kind == TokenKind::OpenParen) { parenDepth++; }
            if (tok.kind == TokenKind::CloseParen) {
                parenDepth--;
                if (parenDepth == 0) {
                    state = FunctionState::ParsingRetType;
                }
            }

            // Very simplified specifier parsing: Key or Key=Value or Key="Value"
            if (tok.kind == TokenKind::Identifier) {
                std::string key = tok.text;
                std::string value = "true";

                // lookahead for '=' Value
                if ((ts.peek(1).kind != TokenKind::EndOfFile) && (ts.peek(1).kind == TokenKind::Assignment)) {
                    if (ts.peek(2).kind != TokenKind::EndOfFile) {
                        value = ts.skip(2).text;
                    }
                }
                current.specifiers[key] = value;
            }
            break;
        }

        case FunctionState::ParsingRetType:
            if (tok.kind == TokenKind::Identifier && tok.text == "virtual") {
                current.isVirtual = true;
                break;
            }
            if (tok.kind == TokenKind::OpenAngle) { angleDepth++; returnTypeTokens.push_back(tok.text); break; }
            if (tok.kind == TokenKind::CloseAngle) { angleDepth--; returnTypeTokens.push_back(tok.text); break; }

            // A ';' or '=' at depth 0 with no name yet is malformed input;
            // an identifier at depth 0 followed by ';'/'=' is the *name*,
            // not part of the type. We detect this by lookahead.
            if (angleDepth == 0 && tok.kind == TokenKind::Identifier) {
                bool nextIsEndOfDecl =
                    ((ts.peek(1).kind != TokenKind::EndOfFile) && (ts.peek(1).kind == TokenKind::OpenParen));

                if (nextIsEndOfDecl) {
                    current.name = tok.text;
                    // Reassemble type tokens (handling * and & spacing
                    // is its own fiddly little problem)
                    current.returnType = joinTokens(returnTypeTokens);
                    state = FunctionState::ParsingFuncArgs;
                }
            }

            returnTypeTokens.push_back(tok.text);
            break;

        case FunctionState::ParsingFuncArgs:
            if (tok.kind == TokenKind::OpenParen) {
                if (!sawArgListOpenParen) {
                    sawArgListOpenParen = true;
                }
                else {
                    parenDepth++; // a nested '(', e.g. std::function<void(
                    argTypeTokens.push_back(tok.text);
                }
                break;
            }

            if (tok.kind == TokenKind::CloseParen) {
                if (parenDepth > 0) {
                    parenDepth--; // closes a nested paren, not the arg list
                    argTypeTokens.push_back(tok.text);
                    break;
                }

                ts.advance();
                if (tryFinishFunction(ts, current)) {
                    state = FunctionState::Done;
                    continue;
                }
                break;
            }

            if (tok.kind == TokenKind::OpenAngle) { angleDepth++; argTypeTokens.push_back(tok.text); break; }
            if (tok.kind == TokenKind::CloseAngle) { angleDepth--; argTypeTokens.push_back(tok.text); break; }

            if (angleDepth == 0 && parenDepth == 0 && tok.kind == TokenKind::Identifier) {
                auto& ahead = ts.peek(1);
                bool isEndOfArgument = ((ahead.kind != TokenKind::EndOfFile) && (ahead.kind == TokenKind::Assignment
                    || ahead.kind == TokenKind::Comma
                    || ahead.kind == TokenKind::CloseParen));

                if (isEndOfArgument) {
                    TokenKind argumentsEndType = ahead.kind;

                    std::string argumentType = joinTokens(argTypeTokens);
                    current.arguments.emplace_back(tok.text, argumentType);

                    argTypeTokens.clear();

                    switch (argumentsEndType) {

                    case TokenKind::Assignment: {
                        ts.skip(2);
                        std::string argValue = "";
                        TokenKind terminator = TokenKind::EndOfFile;

                        while (!ts.atEnd()) {
                            const Token& argToken = ts.advance();
                            if (argToken.kind == TokenKind::Comma || argToken.kind == TokenKind::CloseParen) {
                                terminator = argToken.kind;
                                break;
                            }
                            argValue += argToken.text;
                        }

                        current.defaultArgumentValues.emplace_back(tok.text, argValue);

                        if (terminator == TokenKind::CloseParen) {
                            if (tryFinishFunction(ts, current)) {
                                state = FunctionState::Done;
                            }
                        }
                        break;
                    }

                    case TokenKind::Comma:
                        ts.skip(2);
                        break;

                    case TokenKind::CloseParen:
                        ts.advance();
                        if (tryFinishFunction(ts, current)) {
                            state = FunctionState::Done;
                        }
                        break;

                    default:
                        break;
                    }
                    continue;
                }
            }

            argTypeTokens.push_back(tok.text);
            break;

        case FunctionState::Done:
            return current;
            break;
        }

        ts.advance();
    }
    return current;
}

static ReflectedField parseField(TokenStream& ts) {
    FieldState state = FieldState::Scanning;

    ReflectedField current;
    std::vector<Token> typeTokens;
    int parenDepth = 0;
    int angleDepth = 0;

    while(!ts.atEnd()) {
        const Token& tok = ts.peek();

        switch (state) {

        case FieldState::Scanning:
            if (tok.kind == TokenKind::Identifier) {
                current = ReflectedField{};
                current.line = tok.line;
                typeTokens.clear();

                bool nextIsParen = ((ts.peek(1).kind != TokenKind::EndOfFile) && (ts.peek(1).kind == TokenKind::OpenParen));

                if (nextIsParen) {
                    state = FieldState::ParsingArgs;
                }
            }
            break;

        case FieldState::ParsingArgs: {
            if (tok.kind == TokenKind::OpenParen) { parenDepth++; }
            if (tok.kind == TokenKind::CloseParen) {
                parenDepth--;
                if (parenDepth == 0) {
                    state = FieldState::ParsingDeclType;
                }
            }

            // Very simplified specifier parsing: Key or Key=Value or Key="Value"
            if (tok.kind == TokenKind::Identifier) {
                std::string key = tok.text;
                std::string value = "true";

                // lookahead for '=' Value
                if ((ts.peek(1).kind != TokenKind::EndOfFile) && (ts.peek(1).kind == TokenKind::Assignment)) {
                    if (ts.peek(2).kind != TokenKind::EndOfFile) {
                        value = ts.skip(2).text;
                    }
                }
                current.specifiers[key] = value;
            }
            break;
        }

        case FieldState::ParsingDeclType:
            if (tok.kind == TokenKind::OpenAngle) { angleDepth++; typeTokens.push_back(tok); break; }
            if (tok.kind == TokenKind::CloseAngle) { angleDepth--; typeTokens.push_back(tok); break; }

            // A ';' or '=' at depth 0 with no name yet is malformed input;
            // an identifier at depth 0 followed by ';'/'=' is the *name*,
            // not part of the type. We detect this by lookahead.
            if (angleDepth == 0 && tok.kind == TokenKind::Identifier) {
                bool nextIsEndOfDecl =
                    (ts.peek(1).kind != TokenKind::EndOfFile) &&
                    (ts.peek(1).kind == TokenKind::Semicolon || ts.peek(1).kind == TokenKind::Assignment);

                if (nextIsEndOfDecl) {
                    current.name = tok.text;

                    for (size_t i = 0; i < typeTokens.size(); i++) {
                        Token& typeTok = typeTokens[i];
                        if (typeTok.kind == TokenKind::Identifier && typeTok.text == "const") {
                            current.isConst = true;
                            continue;
                        }
                        current.typeName += typeTok.text;
                    }
                    state = FieldState::ParsingDeclName;
                }
            }

            typeTokens.push_back(tok);
            break;

        case FieldState::ParsingDeclName:
            if (tok.kind == TokenKind::Semicolon) {
                state = FieldState::Done;
            }
            else if (tok.kind == TokenKind::Assignment) {
                ts.skip(2);
                std::string argValue = "";

                while (!ts.atEnd()) {
                    const Token& argToken = ts.advance();
                    if (argToken.kind == TokenKind::CloseParen) {
                        break;
                    }
                    argValue += argToken.text;
                }

                current.value = argValue;
                break;
            }
            break;

        case FieldState::Done:
            return current;
            break;
        }

        ts.advance();
    }
    return current;
}

static ReflectedStruct parseStruct(TokenStream& ts) {
    StructState state = StructState::Scanning;
    ReflectedStruct current;
    std::vector<Token> typeTokens;
    std::string currentMemberName;
    int parenDepth = 0;
    int angleDepth = 0;
    while (!ts.atEnd()) {
        const Token& tok = ts.peek();
        switch (state) {
        case StructState::Scanning:
            if (tok.kind == TokenKind::Identifier) {
                current = ReflectedStruct{};
                typeTokens.clear();
                bool nextIsParen = ((ts.peek(1).kind != TokenKind::EndOfFile) && (ts.peek(1).kind == TokenKind::OpenParen));
                if (nextIsParen) {
                    state = StructState::ParsingArgs;
                }
            }
            break;
        case StructState::ParsingArgs: {
            if (tok.kind == TokenKind::OpenParen) { parenDepth++; }
            if (tok.kind == TokenKind::CloseParen) {
                parenDepth--;
                if (parenDepth == 0) {
                    state = StructState::ParsingStructName;
                }
            }
            if (tok.kind == TokenKind::Identifier) {
                std::string key = tok.text;
                std::string value = "true";
                if ((ts.peek(1).kind != TokenKind::EndOfFile) && (ts.peek(1).kind == TokenKind::Assignment)) {
                    if (ts.peek(2).kind != TokenKind::EndOfFile) {
                        value = ts.skip(2).text;
                    }
                }
                current.specifiers[key] = value;
            }
            break;
        }
        case StructState::ParsingStructName:
            if (tok.kind == TokenKind::Identifier && tok.text == "struct") {
                ts.advance(); // consume "struct"
                if (ts.peek().kind == TokenKind::Identifier && ts.peek().text == "MATRIX_API") {
                    ts.advance();
                }
                current.name = ts.advance().text;
                while (!ts.atEnd() && ts.peek().text != "{") {
                    ts.advance();
                }
                ts.advance(); // consume '{'

                if (!ts.atEnd() && ts.peek().kind == TokenKind::Identifier && ts.peek().text == "REFLECT_STRUCT") {
                    ts.advance(); // consume "REFLECT_STRUCT"
                    if (!ts.atEnd() && ts.peek().kind == TokenKind::OpenParen) {
                        ts.advance(); // consume '('
                        if (!ts.atEnd() && ts.peek().kind == TokenKind::CloseParen) {
                            ts.advance(); // consume ')'
                        }
                    }
                }

                state = StructState::ParsingMemberType;
                continue;
            }
            break;
        case StructState::ParsingMemberType:
            if (tok.kind == TokenKind::OpenAngle) { angleDepth++; typeTokens.push_back(tok); break; }
            if (tok.kind == TokenKind::CloseAngle) { angleDepth--; typeTokens.push_back(tok); break; }
            if (tok.text == "}") {
                state = StructState::Done;
                continue;
            }
            if (angleDepth == 0 && tok.kind == TokenKind::Identifier) {
                TokenKind peek1Kind = ts.peek(1).kind;
                bool nextIsEndOfDecl =
                    (peek1Kind != TokenKind::EndOfFile) &&
                    (peek1Kind == TokenKind::Semicolon || peek1Kind == TokenKind::Assignment);
                if (nextIsEndOfDecl) {
                    std::string memberType;
                    for (auto& t : typeTokens) memberType += t.text;
                    currentMemberName = tok.text;
                    current.members.emplace_back(currentMemberName, memberType);
                    typeTokens.clear();
                    state = StructState::ParsingMemberName;
                    break;
                }
            }
            typeTokens.push_back(tok);
            break;
        case StructState::ParsingMemberName:
            if (tok.kind == TokenKind::Semicolon) {
                state = StructState::ParsingMemberType;
                break;
            }
            else if (tok.kind == TokenKind::Assignment) {
                ts.advance(); // consume '='
                std::string argValue;
                int localParenDepth = 0;
                while (!ts.atEnd()) {
                    const Token& v = ts.peek();
                    if (v.kind == TokenKind::Semicolon && localParenDepth == 0) break;
                    if (v.kind == TokenKind::OpenParen) localParenDepth++;
                    if (v.kind == TokenKind::CloseParen) localParenDepth--;
                    argValue += v.text;
                    ts.advance();
                }
                current.defaultMemberValues.emplace_back(currentMemberName, argValue);
                continue;
            }
            break;
        case StructState::Done:
            return current;
        }
        ts.advance();
    }
    return current;
}
static ReflectedClass parseClass(TokenStream& ts) {
    ReflectedClass cls;
    ts.advance();
    cls.name = ts.advance().text; // class name

    // skip to opening brace (handles ": public Base" in between)
    while (ts.peek().text != "{") {
        const Token& tok = ts.advance();
        if (tok.text == ":") {
            ts.advance();
            cls.parent = ts.advance().text;
        }
    }
    ts.advance(); // consume '{'

    int braceDepth = 1;

    std::string access;

    while (braceDepth > 0 && !ts.atEnd()) {
        if (ts.peek().text == "{") { braceDepth++; ts.advance(); continue; }
        if (ts.peek().text == "}") { braceDepth--; ts.advance(); continue; }

        if (ts.peek().text.compare("public") == 0) {
            access = "public";
        }

        if (ts.peek().text.compare("protected") == 0) {
            access = "protected";
        }

        if (ts.peek().text.compare("private") == 0) {
            access = "private";
        }

        if (ts.peek().text == "FIELD") {
            cls.fields[access].push_back(parseField(ts));
        }
        else if (ts.peek().text == "FUNCTION") {
            cls.functions[access].push_back(parseFunction(ts));
        }
        else {
            ts.advance();
        }
    }
    return cls;
}

static ParsedItems parseFile(TokenStream& ts) {
    ParsedItems items = {};
    
    while (!ts.atEnd()) {
        if (ts.peek().text == "CLASS") {
            ts.skip(4);
            items.classes.push_back(parseClass(ts)); // consumes through the closing brace
        } else if (ts.peek().text == "STRUCT") {
            items.structs.push_back(parseStruct(ts));
        } else {
            ts.advance(); // skip anything at global scope we don't care about yet
        }
    }
    return items;
}

static std::string toIncludePath(const std::string& fullPath) {
    // TODO make it so that marker is changed to Source when reflecting a game
    std::string marker = "Core";

    size_t pos = fullPath.find(marker);
    if (pos == std::string::npos) {
        return std::filesystem::path(fullPath).filename().string();
    }

    std::string relative = fullPath.substr(pos);

    while (!relative.empty() && (relative.front() == '\\' || relative.front() == '/')) {
        relative.erase(relative.begin());
    }

    for (char& c : relative) {
        if (c == '\\') c = '/';
    }

    return relative;
}

static std::vector<Token> tokenize(const std::string& source) {
    std::vector<Token> tokens;
    size_t i = 0, line = 1;

    while (i < source.size()) {
        char c = source[i];

        if (c == '\n') { line++; i++; continue; }
        if (isspace((unsigned char)c)) { i++; continue; }

        // Skip line comments
        if (c == '/' && i + 1 < source.size() && source[i + 1] == '/') {
            while (i < source.size() && source[i] != '\n') i++;
            continue;
        }

        if (c == '/' && i + 1 < source.size() && source[i + 1] == '*') {
            i += 2;
            while (i + 1 < source.size() && !(source[i] == '*' && source[i + 1] == '/')) {
                if (source[i] == '\n') line++;
                i++;
            }
            i += 2;
            continue;
        }

        // Identifiers / keywords
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = i;
            while (i < source.size() && (isalnum((unsigned char)source[i]) || source[i] == '_')) i++;
            tokens.push_back({ TokenKind::Identifier, source.substr(start, i - start), line });
            continue;
        }

        if (isdigit((unsigned char)c)) {
            size_t start = i;
            while (i < source.size() && (isalnum((unsigned char)source[i]) || source[i] == '.')) i++;
            tokens.push_back({ TokenKind::NumberLiteral, source.substr(start, i - start), line });
            continue;
        }

        if (c == '"') {
            size_t start = i++;
            while (i < source.size() && source[i] != '"') i++;
            i++; // consume closing quote
            tokens.push_back({ TokenKind::StringLiteral, source.substr(start, i - start), line });
            continue;
        }

        if (c == '=') {
            size_t start = i;
            tokens.push_back({ TokenKind::Assignment, "=", line });
            i++;
            continue;
        }

        // skipping pragmas and includes, might be a bad idea later on
        if (c == '#') {
            while (i < source.size() && source[i] != '\n') i++;
            continue;
        }

        if (c == ';') {
            size_t start = i;
            tokens.push_back({ TokenKind::Semicolon, ";", line});
            i++;
            continue;
        }

        if (c == ':') {
            if (i + 1 < source.size() && source[i + 1] == ':') {
                tokens.push_back({ TokenKind::Punctuation, "::", line });
                i += 2;
                continue;
            }
            tokens.push_back({ TokenKind::Punctuation, ":", line });
            i++;
            continue;
        }

        if (c == '(') {
            size_t start = i;
            tokens.push_back({ TokenKind::OpenParen, "(", line });
            i++;
            continue;
        }

        if (c == ')') {
            size_t start = i;
            tokens.push_back({ TokenKind::CloseParen, ")", line });
            i++;
            continue;
        }

        if (c == '<') {
            size_t start = i;
            tokens.push_back({ TokenKind::OpenAngle, "<", line });
            i++;
            continue;
        }

        if (c == '>') {
            size_t start = i;
            tokens.push_back({ TokenKind::CloseAngle, ">", line });
            i++;
            continue;
        }

        if (c == ',') {
            size_t start = i;
            tokens.push_back({ TokenKind::Comma, ",", line });
            i++;
            continue;
        }

        // Everything else: punctuation, one char at a time
        // (good enough for *, &, ::, etc.)
        tokens.push_back({ TokenKind::Punctuation, std::string(1, c), line });
        i++;
    }

    tokens.push_back({ TokenKind::EndOfFile, "", line });
    return tokens;
}

static void writeStruct(ReflectedStruct& rs, char* filepath, std::string outputDir) {
    std::string& structName = rs.name;

    std::ofstream hppFile(outputDir + "/" + structName + ".reflected.hpp");
    hppFile << "#pragma once\n";
    hppFile << "#include \"Core/Structs/ReflectionStructs.hpp\"\n";
    hppFile << "#include <cstring>\n\n";
    hppFile << "#ifdef REFLECT_STRUCT\n";
    hppFile << "#undef REFLECT_STRUCT\n";
    hppFile << "#endif\n\n";
    hppFile << "#define REFLECT_STRUCT() \\\n";
    hppFile << "public: \\\n";
    hppFile << "\tstatic const ReflectedStruct& StaticStruct(); \\\n";

   /* hppFile << "\tinline void GetStructMemberValue(const ReflectedStruct& structInfo, const std::string& memberName, const void* obj, void* out) { \\\n";
    hppFile << "\t\tsize_t offset = structInfo.memberOffsets.at(memberName); \\\n";
    hppFile << "\t\tsize_t size = structInfo.memberSizes.at(memberName); \\\n";
    hppFile << "\t\tconst void* fieldPtr = static_cast<const uint8_t*>(obj) + offset; \\\n";
    hppFile << "\t\tmemcpy(out, fieldPtr, size); \\\n";
    hppFile << "\t} \\\n";
    
    hppFile << "\tinline void SetStructMemberValue(const ReflectedStruct& structInfo, const std::string& memberName, void* obj, const void* in) { \\\n";
    hppFile << "\t\tsize_t offset = structInfo.memberOffsets.at(memberName); \\\n";
    hppFile << "\t\tsize_t size = structInfo.memberSizes.at(memberName); \\\n";
    hppFile << "\t\tvoid* fieldPtr = static_cast<uint8_t*>(obj) + offset; \\\n";
    hppFile << "\t\tmemcpy(fieldPtr, in, size); \\\n";
    hppFile << "\t} \\";*/
    hppFile.close();

    std::ofstream cppFile(outputDir + "/" + structName + ".reflected.cpp");
    cppFile << "#include \"" << toIncludePath(filepath) << "\"\n\n";
    cppFile << "const ReflectedStruct& " << structName << "::StaticStruct() {\n";
    cppFile << "\tstatic ReflectedStruct info = []() {\n";
    cppFile << "\t\tReflectedStruct s;\n";
    cppFile << "\t\ts.name = \"" << structName << "\";\n";
    cppFile << "\t\ts.size = sizeof(" << structName << ");\n\n";

    for (auto& [memberName, memberType] : rs.members) {
        cppFile << "\t\ts.members.emplace_back(\"" << memberName << "\", \"" << memberType << "\");\n";
        cppFile << "\t\ts.memberOffsets[\"" << memberName << "\"] = offsetof(" << structName << ", " << memberName << ");\n";
        cppFile << "\t\ts.memberSizes[\"" << memberName << "\"] = sizeof(" << memberType << ");\n\n";
    }

    for (auto& [memberName, defaultValue] : rs.defaultMemberValues) {
        cppFile << "\t\ts.defaultMemberValues.emplace_back(\"" << memberName << "\", \"" << defaultValue << "\");\n";
    }

    cppFile << "\n\t\treturn s;\n";
    cppFile << "\t}();\n";
    cppFile << "\treturn info;\n";
    cppFile << "}\n";
    cppFile.close();
}

static void writeClass(ReflectedClass& rc, char* filepath, std::string outputDir) {
    std::string& className = rc.name;
    std::array ar = { "public", "protected", "private" };

    // Compute thunk names
    std::map<std::string, std::vector<std::string>> thunkNamesByAccess;
    for (auto& access : ar) {
        std::map<std::string, int> nameCounts;
        for (auto& func : rc.functions[access]) {
            int overloadIndex = nameCounts[func.name]++;
            std::string thunkName = func.name + "_Thunk";
            if (overloadIndex > 0) {
                thunkName += "_" + std::to_string(overloadIndex);
            }
            thunkNamesByAccess[access].push_back(thunkName);
        }
    }

    std::ofstream hppFile(outputDir + "/" + className + ".reflected.hpp");
    hppFile << "#pragma once\n";
    hppFile << "#include \"Core/Structs/ReflectionStructs.hpp\"\n\n";
    hppFile << "#ifdef REFLECTION\n";
    hppFile << "#undef REFLECTION\n";
    hppFile << "#endif\n\n";
    hppFile << "#define REFLECTION() \\\n";
    hppFile << "public: \\\n";
    hppFile << "\tstatic const ReflectedClass& StaticClass(); \\\n";
    if (rc.parent == "None") {
        hppFile << "\tvirtual const ReflectedClass& GetClass() const = 0; \\\n";
        
        hppFile << "\tinline void GetFieldValue(const ReflectedField& field, const void* obj, void* out) { \\\n";
        hppFile << "\t\tconst void* fieldPtr = static_cast<const uint8_t*>(obj) + field.offset; \\\n";
        hppFile << "\t\tmemcpy(out, fieldPtr, field.size); \\\n";
        hppFile << "\t} \\\n";

        hppFile << "\tinline void SetFieldValue(const ReflectedField& field, void* obj, const void* in) { \\\n";
        hppFile << "\t\tif (field.isConst) { \\\n";
        hppFile << "\t\t\treturn; \\\n";
        hppFile << "\t\t} \\\n";
        hppFile << "\t\tvoid* fieldPtr = static_cast<uint8_t*>(obj) + field.offset; \\\n";
        hppFile << "\t\tmemcpy(fieldPtr, in, field.size); \\\n";
        hppFile << "\t} \\\n";
    }
    else {
        hppFile << "\tvirtual const ReflectedClass& GetClass() const override; \\\n";
    }

    for (auto& access : ar) {
        auto& thunkNames = thunkNamesByAccess[access];
        for (auto& thunkName : thunkNames) {
            hppFile << "\tstatic void " << thunkName << "(void* obj, void** args, void* outReturn); \\\n";
        }
    }
    hppFile.close();

    /*std::string filePathString = filepath;
    std::cout << "LOOK HERE" << filePathString << std::endl;*/

    std::ofstream cppFile(outputDir + "/" + className + ".reflected.cpp");
    cppFile << "#include \"" << toIncludePath(filepath) << "\"\n\n";
    cppFile << "const ReflectedClass& " << className << "::GetClass() const { return " << className << "::StaticClass(); }\n\n";

    for (auto& access : ar) {
        auto& thunkNames = thunkNamesByAccess[access];
        for (size_t idx = 0; idx < rc.functions[access].size(); idx++) {
            auto& func = rc.functions[access][idx];
            const std::string& thunkName = thunkNames[idx];

            cppFile << "void " << className << "::" << thunkName << "(void* obj, void** args, void* outReturn) {\n";
            for (size_t i = 0; i < func.arguments.size(); i++) {
                auto& [argName, argType] = func.arguments[i];

                std::string paramType = argType;
                bool isReference = !paramType.empty() && paramType.back() == '&';
                if (isReference) {
                    paramType.pop_back();
                    // trim a possible trailing space left behind (e.g. "std::string &" -> "std::string")
                    while (!paramType.empty() && paramType.back() == ' ') {
                        paramType.pop_back();
                    }
                }

                if (isReference) {
                    cppFile << "\t" << paramType << "& " << argName << " = *static_cast<" << paramType << "*>(args[" << i << "]);\n";
                }
                else {
                    cppFile << "\t" << argType << " " << argName << " = *static_cast<" << argType << "*>(args[" << i << "]);\n";
                }
            }
            cppFile << "\t";
            if (func.returnType != "void") {
                cppFile << "*static_cast<" << func.returnType << "*>(outReturn) = ";
            }
            cppFile << "static_cast<" << className << "*>(obj)->" << func.name << "(";
            for (size_t i = 0; i < func.arguments.size(); i++) {
                cppFile << func.arguments[i].first;
                if (i + 1 < func.arguments.size()) cppFile << ", ";
            }
            cppFile << ");\n";
            cppFile << "}\n\n";
        }
    }
    
    cppFile << "const ReflectedClass& " << className << "::StaticClass() {\n";
    cppFile << "\tstatic ReflectedClass info = []() {\n";
    cppFile << "\t\tReflectedClass c;\n";
    cppFile << "\t\tc.name = \"" << className << "\";\n";
    cppFile << "\t\tc.parent = \"" << rc.parent << "\";\n";
    cppFile << "\t\tc.size = " << rc.size << ";\n\n";

    for (auto& access : ar) {
        auto& thunkNames = thunkNamesByAccess[access];
        for (auto& field : rc.fields[access]) {
            cppFile << "\t\tc.fields[\"" << access << "\"].push_back(ReflectedField{\n";
            cppFile << "\t\t\t.name = \"" << field.name << "\",\n";
            cppFile << "\t\t\t.typeName = \"" << field.typeName << "\",\n";
            cppFile << "\t\t\t.isConst = " << std::boolalpha << field.isConst << ",\n";
            cppFile << "\t\t\t.offset = offsetof(" << className << ", " << field.name << "),\n";
            cppFile << "\t\t\t.size = sizeof(" << field.typeName << ")\n";
            cppFile << "\t\t});\n\n";
        }
        for (size_t idx = 0; idx < rc.functions[access].size(); idx++) {
            auto& func = rc.functions[access][idx];
            const std::string& thunkName = thunkNames[idx];
            
            cppFile << "\t\tc.functions[\"" << access << "\"].push_back(ReflectedFunction{\n";
            cppFile << "\t\t\t.name = \"" << func.name << "\",\n";
            cppFile << "\t\t\t.returnType = \"" << func.returnType << "\",\n";
            cppFile << "\t\t\t.arguments = {";
            if (func.arguments.size() > 0) {
                cppFile << "\n";
                for (size_t i = 0; i < func.arguments.size(); i++) {
                    cppFile << "\t\t\t\t{ \"" << func.arguments[i].first << "\", \"" << func.arguments[i].second << "\" }";
                    if (i + 1 < func.arguments.size()) cppFile << ",\n";
                }
                cppFile << "\n\t\t\t";
            }
            cppFile << "},\n";
            cppFile << "\t\t\t.defaultArgumentValues = {";
            if (func.defaultArgumentValues.size() > 0) {
                cppFile << "\n";
                for (size_t i = 0; i < func.defaultArgumentValues.size(); i++) {
                    cppFile << "\t\t\t\t{ \"" << func.defaultArgumentValues[i].first << "\", \"" << func.defaultArgumentValues[i].second << "\" }";
                    if (i + 1 < func.defaultArgumentValues.size()) cppFile << ",\n";
                }
                cppFile << "\n\t\t\t";
            }
            cppFile << "},\n";
            cppFile << "\t\t\t.invoke = &" << className << "::" << thunkName << "\n";
            cppFile << "\t\t});\n\n";
        }
    }

    cppFile << "\t\treturn c;\n";
    cppFile << "\t}();\n";
    cppFile << "\treturn info;\n";
    cppFile << "}\n";
    cppFile.close();
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: Reflector <output_dir> <file1.hpp> <file2.hpp> ...\n";
        return 1;
    }

    std::string outputDir = argv[1];
    
    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directory(outputDir);
    }

    for (int i = 2; i < argc; i++) {
        std::ifstream f(argv[i]);

        if (!f) {
            std::cout << "Error opening file: " << argv[i] << "\n";
            continue;
        }

        std::string contents(
            (std::istreambuf_iterator<char>(f)),
            std::istreambuf_iterator<char>()
        );

        f.close();

        /*std::cout << contents;*/
        TokenStream ts{
            .tokens = tokenize(contents)
        };

        /*for (auto& tok : ts.tokens) {
            std::cout << tok.text << std::endl;
        }*/

        ParsedItems items = parseFile(ts);

        for (auto& rc : items.classes) {
            writeClass(rc, argv[i], outputDir);
        }
        for (auto& rs : items.structs) {
            writeStruct(rs, argv[i], outputDir);
        }
    }

    return 0;
}