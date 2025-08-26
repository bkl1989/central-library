#ifndef CL_PARSER_PARSER_PARSER_HPP
#define CL_PARSER_PARSER_PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <stack>

std::string char32ToUtf8(char32_t ch);
std::u32string UTF8toUTF32(const std::string& input);
std::string U32StringToString(const std::u32string& input);

class ParserNode {
public:
    std::string toString() const;
    std::string toString(std::string indentation) const;
    ParserNode(const std::string &n);
    ParserNode(char32_t n);
    ~ParserNode();
    ParserNode *getParent ();
    ParserNode *createChild (const std::string &n);
    ParserNode *createChild (char32_t n);
    bool addCharacter (char32_t characterToAdd);
    ParserNode *lastChild ();
    std::u32string getValue ();
};

struct ParserResult {
    ParserNode *node;
    std::string error;
};

class CharacterSet {
public:
    virtual std::vector<char32_t> *getCharacters ();
};

class StringCharacterSet : public CharacterSet {
public:
    StringCharacterSet(const std::string& str);
    ~StringCharacterSet();
    std::vector<char32_t> *getCharacters ();
};

class SubGrammarComponent {
    public:
    virtual ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    virtual std::string toString() const;
};

class CompositeSubGrammarComponent : public SubGrammarComponent {
public:
    CompositeSubGrammarComponent ();
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    virtual std::string toString() const;
    bool addSubGrammarComponent (SubGrammarComponent &componentToAdd);
};

class PushSubGrammarComponent : public SubGrammarComponent {
    public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class PopSubGrammarComponent : public SubGrammarComponent {
    public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class PushNameSubGrammarComponent : public SubGrammarComponent {
    public:
    PushNameSubGrammarComponent(std::string n);
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class PopNameSubGrammarComponent : public SubGrammarComponent {
    public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class NoOpSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class ErrorSubGrammarComponent : public SubGrammarComponent {
public:
    ErrorSubGrammarComponent(std::string m);
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class SubGrammar {
protected:
    SubGrammarComponent *defaultComponent = nullptr;
public:
    SubGrammar ();
    ~SubGrammar ();
    bool addComponent(CharacterSet *onChars, SubGrammarComponent *component);
    std::unordered_map<char32_t, SubGrammarComponent *> asUnorderedMap ();
    bool setDefaultComponent (SubGrammarComponent &c);
};

// Formats subGrammar data so that it can be most efficiently used for parsing
class SubGrammarParser {
public:
    SubGrammarParser(SubGrammar &forSubGrammar);
    ~SubGrammarParser();
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
};

class GrammarParser {
public:
    GrammarParser();
    ~GrammarParser();
    bool addSubGrammarParser (std::string name, SubGrammarParser *parser);
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
};

#endif