#include <filesystem>
#include <fstream>
#include <array>
#include <iostream>
#include "../Core/Structs/ReflectionStructs.hpp"

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
    std::vector<Class> classes;
    std::vector<Struct> structs;
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

static std::string Trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

static std::shared_ptr<TypeInfo> ParseTypeInfoFromString(const std::string& raw) {
    static const std::vector<std::string> mapTypeNames = { "HashMap" };

    for (auto& mapName : mapTypeNames) {
        std::string prefix = mapName + "<";
        if (raw.size() > prefix.size() && raw.compare(0, prefix.size(), prefix) == 0 && raw.back() == '>') {
            std::string inner = raw.substr(prefix.size(), raw.size() - prefix.size() - 1);

            int depth = 0;
            size_t splitPos = std::string::npos;
            for (size_t i = 0; i < inner.size(); i++) {
                if (inner[i] == '<') depth++;
                else if (inner[i] == '>') depth--;
                else if (inner[i] == ',' && depth == 0) { splitPos = i; break; }
            }

            if (splitPos != std::string::npos) {
                std::string keyStr = Trim(inner.substr(0, splitPos));
                std::string valStr = Trim(inner.substr(splitPos + 1));

                auto mapInfo = std::make_shared<MapTypeInfo>();
                mapInfo->typeName = mapName + "<" + keyStr + "," + valStr + ">";
                mapInfo->keyInfo = ParseTypeInfoFromString(keyStr);
                mapInfo->valInfo = ParseTypeInfoFromString(valStr);
                return mapInfo;
            }
        }
    }

    auto info = std::make_shared<TypeInfo>();
    info->typeName = Trim(raw);
    return info;
}

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

static bool tryFinishFunction(TokenStream& ts, Function& current) {
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

static Function parseFunction(TokenStream& ts) {
    FunctionState state = FunctionState::Scanning;

    Function current;
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
                current = Function{};
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
                    current.funcName = tok.text;

                    // Split "const" out of the collected return-type tokens,
                    // same approach as parseField uses for field types.
                    std::vector<std::string> filteredTokens;
                    bool retIsConst = false;
                    for (auto& t : returnTypeTokens) {
                        if (t == "const") { retIsConst = true; continue; }
                        filteredTokens.push_back(t);
                    }
                    std::string retTypeStr = joinTokens(filteredTokens);
                    current.returnTypeInfo = ParseTypeInfoFromString(retTypeStr);
                    current.returnTypeInfo->isConst = retIsConst;

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

                    std::vector<std::string> filteredTokens;
                    bool argIsConst = false;
                    for (auto& t : argTypeTokens) {
                        if (t == "const") {
                            argIsConst = true;
                            continue;
                        }
                        filteredTokens.push_back(t);
                    }
                    std::string typeStr = joinTokens(filteredTokens);

                    std::shared_ptr<TypeInfo> info = ParseTypeInfoFromString(typeStr);
                    info->isConst = argIsConst;
                    current.arguments.emplace_back(tok.text, info);

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

static Field parseField(TokenStream& ts) {
    FieldState state = FieldState::Scanning;

    Field current;
    std::vector<Token> typeTokens;
    int parenDepth = 0;
    int angleDepth = 0;

    while(!ts.atEnd()) {
        const Token& tok = ts.peek();

        switch (state) {

        case FieldState::Scanning:
            if (tok.kind == TokenKind::Identifier) {
                current = Field{};
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
                    current.fieldName = tok.text;

                    std::vector<std::string> filteredTokens;
                    for (auto& typeTok : typeTokens) {
                        if (typeTok.kind == TokenKind::Identifier && typeTok.text == "const") {
                            current.typeInfo->isConst = true;
                            continue;
                        }
                        filteredTokens.push_back(typeTok.text);
                    }
                    std::string typeStr = joinTokens(filteredTokens);

                    current.typeInfo = ParseTypeInfoFromString(typeStr);

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

static Struct parseStruct(TokenStream& ts) {
    StructState state = StructState::Scanning;
    Struct current;
    std::vector<Token> typeTokens;
    std::string currentMemberName;
    int parenDepth = 0;
    int angleDepth = 0;
    while (!ts.atEnd()) {
        const Token& tok = ts.peek();
        switch (state) {
        case StructState::Scanning:
            if (tok.kind == TokenKind::Identifier) {
                current = Struct{};
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
                current.typeInfo.get()->typeName = ts.advance().text;
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
                    std::string memberTypeStr;
                    for (auto& t : typeTokens) memberTypeStr += t.text;

                    currentMemberName = tok.text;
                    current.members.emplace_back(currentMemberName, ParseTypeInfoFromString(memberTypeStr));

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

static Class parseClass(TokenStream& ts) {
    Class cls;
    ts.advance();
    cls.typeInfo.get()->typeName = ts.advance().text; // class name

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

static int g_mapThunkCounter = 0; // ensures unique thunk names across the whole file

static std::string EmitTypeInfoConstruction(std::ofstream& cppFile, const std::shared_ptr<TypeInfo>& info) {
    if (auto mapInfo = std::dynamic_pointer_cast<MapTypeInfo>(info)) {
        std::string keyExpr = EmitTypeInfoConstruction(cppFile, mapInfo->keyInfo);
        std::string valExpr = EmitTypeInfoConstruction(cppFile, mapInfo->valInfo);

        std::string keyType = mapInfo->keyInfo->typeName;
        std::string valType = mapInfo->valInfo->typeName;
        std::string thunkName = "MapForEach_Thunk_" + std::to_string(g_mapThunkCounter++);

        cppFile << "static void " << thunkName
            << "(void* mapObj, void (*visit)(const void* keyPtr, void* valPtr, void* userdata), void* userdata) {\n";
        cppFile << "\tauto* map = static_cast<HashMap<" << keyType << ", " << valType << ">*>(mapObj);\n";
        cppFile << "\tfor (auto& [k, v] : *map) {\n";
        cppFile << "\t\tvisit(&k, &v, userdata);\n";
        cppFile << "\t}\n";
        cppFile << "}\n\n";

        std::ostringstream oss;
        oss << "[]() {\n";
        oss << "\t\t\t\tauto m = std::make_shared<MapTypeInfo>();\n";
        oss << "\t\t\t\tm->typeName = \"" << mapInfo->typeName << "\";\n";
        oss << "\t\t\t\tm->isConst = " << std::boolalpha << mapInfo->isConst << ";\n";
        oss << "\t\t\t\tm->size = sizeof(" << mapInfo->typeName << ");\n";
        oss << "\t\t\t\tm->keyInfo = " << keyExpr << ";\n";
        oss << "\t\t\t\tm->valInfo = " << valExpr << ";\n";
        oss << "\t\t\t\tm->forEachEntry = &" << thunkName << ";\n";
        oss << "\t\t\t\treturn m;\n";
        oss << "\t\t\t}()";
        return oss.str();
    }

    std::ostringstream oss;
    oss << "[]() {\n";
    oss << "\t\t\t\tauto t = std::make_shared<TypeInfo>();\n";
    oss << "\t\t\t\tt->typeName = \"" << info->typeName << "\";\n";
    oss << "\t\t\t\tt->isConst = " << std::boolalpha << info->isConst << ";\n";
    if (info->typeName == "void") {
        oss << "\t\t\t\tt->size = 0;\n";
    }
    else {
        oss << "\t\t\t\tt->size = sizeof(" << info->typeName << ");\n";
    }
    oss << "\t\t\t\treturn t;\n";
    oss << "\t\t\t}()";
    return oss.str();
}

static void writeStruct(Struct& rs, char* filepath, std::string outputDir) {
    std::string structName = rs.typeInfo->typeName;

    std::ofstream hppFile(outputDir + "/" + structName + ".reflected.hpp");
    hppFile << "#pragma once\n";
    hppFile << "#include \"Core/Structs/ReflectionStructs.hpp\"\n";
    hppFile << "#include <cstring>\n\n";
    hppFile << "#ifdef REFLECT_STRUCT\n";
    hppFile << "#undef REFLECT_STRUCT\n";
    hppFile << "#endif\n\n";
    hppFile << "#define REFLECT_STRUCT() \\\n";
    hppFile << "public: \\\n";
    hppFile << "\tstatic const Struct& StaticStruct(); \\\n";
    hppFile.close();

    std::ofstream cppFile(outputDir + "/" + structName + ".reflected.cpp");
    cppFile << "#include \"Core/TypeRegistry/TypeRegistry.hpp\"\n";
    cppFile << "#include \"" << toIncludePath(filepath) << "\"\n\n";

    // PASS 1: resolve member type expressions first (emits any needed thunks)
    std::vector<std::string> memberTypeExprs;
    for (auto& [memberName, memberTypeInfo] : rs.members) {
        memberTypeExprs.push_back(EmitTypeInfoConstruction(cppFile, memberTypeInfo));
    }

    // PASS 2: write StaticStruct() using the pre-resolved strings
    cppFile << "const Struct& " << structName << "::StaticStruct() {\n";
    cppFile << "\tstatic Struct info = []() {\n";
    cppFile << "\t\tStruct s(\"" << structName << "\", sizeof(" << structName << "));\n\n";

    for (size_t i = 0; i < rs.members.size(); i++) {
        auto& [memberName, memberTypeInfo] = rs.members[i];
        cppFile << "\t\ts.members.emplace_back(\"" << memberName << "\", " << memberTypeExprs[i] << ");\n";
        cppFile << "\t\ts.memberOffsets[\"" << memberName << "\"] = offsetof(" << structName << ", " << memberName << ");\n\n";
    }

    for (auto& [memberName, defaultValue] : rs.defaultMemberValues) {
        cppFile << "\t\ts.defaultMemberValues.emplace_back(\"" << memberName << "\", \"" << defaultValue << "\");\n";
    }

    cppFile << "\n\t\treturn s;\n";
    cppFile << "\t}();\n";
    cppFile << "\treturn info;\n";
    cppFile << "}\n";

    cppFile << "\n";
    cppFile << "namespace {\n";
    cppFile << "\tconst bool " << structName << "_typeRegistered = TypeRegistry::RegisterType(\"" << structName << "\", &" << structName << "::StaticStruct());\n";
    cppFile << "}\n";

    cppFile.close();
}

static void writeClass(Class& rc, char* filepath, std::string outputDir) {
    std::string className = rc.typeInfo->typeName;
    std::array<std::string, 3> ar = { "public", "protected", "private" };

    std::map<std::string, std::vector<std::string>> thunkNamesByAccess;
    for (auto& access : ar) {
        std::map<std::string, int> nameCounts;
        for (auto& func : rc.functions[access]) {
            int overloadIndex = nameCounts[func.funcName]++;
            std::string thunkName = func.funcName + "_Thunk";
            if (overloadIndex > 0) {
                thunkName += "_" + std::to_string(overloadIndex);
            }
            thunkNamesByAccess[access].push_back(thunkName);
        }
    }

    std::ofstream hppFile(outputDir + "/" + className + ".reflected.hpp");
    hppFile << "#pragma once\n";
    hppFile << "#include \"Core/Structs/ReflectionStructs.hpp\"\n\n";
    hppFile << "#ifdef REFLECT_CLASS\n";
    hppFile << "#undef REFLECT_CLASS\n";
    hppFile << "#endif\n\n";
    hppFile << "#define REFLECT_CLASS() \\\n";
    hppFile << "public: \\\n";
    hppFile << "\tstatic const Class& StaticClass(); \\\n";
    if (rc.parent == "None") {
        hppFile << "\tvirtual const Class& GetClass() const = 0; \\\n";

        hppFile << "\tinline void GetFieldValue(const Field& field, const void* obj, void* out) { \\\n";
        hppFile << "\t\tconst void* fieldPtr = static_cast<const uint8_t*>(obj) + field.offset; \\\n";
        hppFile << "\t\tmemcpy(out, fieldPtr, field.typeInfo->size); \\\n";
        hppFile << "\t} \\\n";

        hppFile << "\tinline void SetFieldValue(const Field& field, void* obj, const void* in) { \\\n";
        hppFile << "\t\tif (field.typeInfo->isConst) { \\\n";
        hppFile << "\t\t\treturn; \\\n";
        hppFile << "\t\t} \\\n";
        hppFile << "\t\tvoid* fieldPtr = static_cast<uint8_t*>(obj) + field.offset; \\\n";
        hppFile << "\t\tmemcpy(fieldPtr, in, field.typeInfo->size); \\\n";
        hppFile << "\t} \\\n";
    }
    else {
        hppFile << "\tvirtual const Class& GetClass() const override; \\\n";
    }

    for (auto& access : ar) {
        auto& thunkNames = thunkNamesByAccess[access];
        for (auto& thunkName : thunkNames) {
            hppFile << "\tstatic void " << thunkName << "(void* obj, void** args, void* outReturn); \\\n";
        }
    }
    hppFile.close();

    std::ofstream cppFile(outputDir + "/" + className + ".reflected.cpp");
    cppFile << "#include \"Core/TypeRegistry/TypeRegistry.hpp\"\n";
    cppFile << "#include \"" << toIncludePath(filepath) << "\"\n\n";
    cppFile << "const Class& " << className << "::GetClass() const { return " << className << "::StaticClass(); }\n\n";

    for (auto& access : ar) {
        auto& thunkNames = thunkNamesByAccess[access];
        for (size_t idx = 0; idx < rc.functions[access].size(); idx++) {
            auto& func = rc.functions[access][idx];
            const std::string& thunkName = thunkNames[idx];

            cppFile << "void " << className << "::" << thunkName << "(void* obj, void** args, void* outReturn) {\n";
            for (size_t i = 0; i < func.arguments.size(); i++) {
                auto& [argName, argTypeInfo] = func.arguments[i];
                std::string argType = argTypeInfo->typeName;

                std::string paramType = argType;
                bool isReference = !paramType.empty() && paramType.back() == '&';
                if (isReference) {
                    paramType.pop_back();
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
            if (func.returnTypeInfo->typeName != "void") {
                cppFile << "*static_cast<" << func.returnTypeInfo->typeName << "*>(outReturn) = ";
            }
            cppFile << "static_cast<" << className << "*>(obj)->" << func.funcName << "(";
            for (size_t i = 0; i < func.arguments.size(); i++) {
                cppFile << func.arguments[i].first;
                if (i + 1 < func.arguments.size()) cppFile << ", ";
            }
            cppFile << ");\n";
            cppFile << "}\n\n";
        }
    }

    // --- PASS 1: resolve every TypeInfo construction string up front.
    // This is what forces all map thunks to be written to cppFile NOW,
    // before StaticClass()'s body starts.
    std::map<std::string, std::string> fieldTypeExprs; // keyed by access + ":" + fieldName
    std::map<std::string, std::string> funcReturnTypeExprs; // keyed by access + ":" + idx
    std::map<std::string, std::vector<std::string>> funcArgTypeExprs; // keyed by access + ":" + idx

    for (auto& access : ar) {
        for (auto& field : rc.fields[access]) {
            fieldTypeExprs[access + ":" + field.fieldName] = EmitTypeInfoConstruction(cppFile, field.typeInfo);
        }
        for (size_t idx = 0; idx < rc.functions[access].size(); idx++) {
            auto& func = rc.functions[access][idx];
            std::string key = access + ":" + std::to_string(idx);
            funcReturnTypeExprs[key] = EmitTypeInfoConstruction(cppFile, func.returnTypeInfo);

            std::vector<std::string> argExprs;
            for (auto& [argName, argTypeInfo] : func.arguments) {
                argExprs.push_back(EmitTypeInfoConstruction(cppFile, argTypeInfo));
            }
            funcArgTypeExprs[key] = argExprs;
        }
    }

    // --- PASS 2: now write StaticClass(), using the pre-resolved strings.
    cppFile << "const Class& " << className << "::StaticClass() {\n";
    cppFile << "\tstatic Class info = []() {\n";
    cppFile << "\t\tClass c(\"" << className << "\", sizeof(" << className << "));\n";
    cppFile << "\t\tc.parent = \"" << rc.parent << "\";\n\n";

    for (auto& access : ar) {
        auto& thunkNames = thunkNamesByAccess[access];
        for (auto& field : rc.fields[access]) {
            cppFile << "\t\tc.fields[\"" << access << "\"].push_back(Field{\n";
            cppFile << "\t\t\t.fieldName = \"" << field.fieldName << "\",\n";
            cppFile << "\t\t\t.typeInfo = " << fieldTypeExprs[access + ":" + field.fieldName] << ",\n";
            cppFile << "\t\t\t.offset = offsetof(" << className << ", " << field.fieldName << "),\n";
            cppFile << "\t\t});\n\n";
        }
        for (size_t idx = 0; idx < rc.functions[access].size(); idx++) {
            auto& func = rc.functions[access][idx];
            const std::string& thunkName = thunkNames[idx];
            std::string key = access + ":" + std::to_string(idx);

            cppFile << "\t\tc.functions[\"" << access << "\"].push_back(Function{\n";
            cppFile << "\t\t\t.funcName = \"" << func.funcName << "\",\n";
            cppFile << "\t\t\t.returnTypeInfo = " << funcReturnTypeExprs[key] << ",\n";
            cppFile << "\t\t\t.arguments = {";
            if (func.arguments.size() > 0) {
                cppFile << "\n";
                auto& argExprs = funcArgTypeExprs[key];
                for (size_t i = 0; i < func.arguments.size(); i++) {
                    cppFile << "\t\t\t\t{ \"" << func.arguments[i].first << "\", " << argExprs[i] << " }";
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
            cppFile << "\t\t\t.invoke = &" << className << "::" << thunkName << ",\n";
            cppFile << "\t\t\t.isVirtual = " << std::boolalpha << func.isVirtual << ",\n";
            cppFile << "\t\t\t.line = " << func.line << "\n";
            cppFile << "\t\t});\n\n";
        }
    }

    cppFile << "\t\treturn c;\n";
    cppFile << "\t}();\n";
    cppFile << "\treturn info;\n";
    cppFile << "}\n";

    cppFile << "\n";
    cppFile << "namespace {\n";
    cppFile << "\tconst bool " << className << "_typeRegistered = TypeRegistry::RegisterType(\"" << className << "\", &" << className << "::StaticClass());\n";
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