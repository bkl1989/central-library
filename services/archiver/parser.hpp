#ifndef CL_PARSER_PARSER_HPP
#define CL_PARSER_PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <unordered_set>
#include <stack>

std::string char32ToUtf8(char32_t ch);
std::u32string UTF8toUTF32(const std::string& input);
std::string U32StringToString(const std::u32string& input);

class ParserNode {
    std::u32string *text;
    std::vector<ParserNode *> *children;
    ParserNode *parent = NULL;
public:
    std::string toString() const;
    std::string toString(std::string indentation) const;
    ParserNode(const std::string &n);
    ParserNode(char32_t n);
    ParserNode();
    ~ParserNode();
    ParserNode *getParent ();
    ParserNode *createChild (const std::string &n);
    ParserNode *createChild (char32_t n);
    bool addCharacter (char32_t characterToAdd);
    ParserNode *lastChild ();
    ParserNode *getChild (int index);
    std::u32string getValue ();
    size_t size();
};

struct ParserResult {
    ParserNode *node;
    std::string error;
};

class CharacterSet {
public:
    virtual std::vector<char32_t> *getCharacters ();
    virtual ~CharacterSet();
    CharacterSet();
};

class StringCharacterSet : public CharacterSet {
    std::vector<char32_t> *characters;
public:
    StringCharacterSet(const std::string& str);
    StringCharacterSet();
    ~StringCharacterSet();
    std::vector<char32_t> *getCharacters ();
};

class SubGrammarComponent {
    public:
    virtual ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    virtual std::string toString() const;
    SubGrammarComponent();
    virtual ~SubGrammarComponent();
};

class CompositeSubGrammarComponent : public SubGrammarComponent {
    std::vector<SubGrammarComponent *> *subGrammarComponents;
public:
    CompositeSubGrammarComponent ();
    ~CompositeSubGrammarComponent ();
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    bool addSubGrammarComponent (SubGrammarComponent &componentToAdd);
};

class NoSiblingsSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    NoSiblingsSubGrammarComponent();
    ~NoSiblingsSubGrammarComponent();
};


class NoEmptySiblingsSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    NoEmptySiblingsSubGrammarComponent();
    ~NoEmptySiblingsSubGrammarComponent();
};

class NoEmptyValueSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    NoEmptyValueSubGrammarComponent();
    ~NoEmptyValueSubGrammarComponent();
};

class NewSiblingSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    NewSiblingSubGrammarComponent();
    ~NewSiblingSubGrammarComponent();
};

class PushSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    PushSubGrammarComponent();
    ~PushSubGrammarComponent();
};

class PopSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    PopSubGrammarComponent();
    ~PopSubGrammarComponent();
};

class PushNameSubGrammarComponent : public SubGrammarComponent {
    std::string name;
public:
    PushNameSubGrammarComponent();
    ~PushNameSubGrammarComponent();
    PushNameSubGrammarComponent(std::string n);
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class UnlessEscapedSubGrammarComponent : public SubGrammarComponent {
    SubGrammarComponent *component, *escapedComponent;
    std::unordered_set<char32_t> *escapeCharacters;
public:
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
    UnlessEscapedSubGrammarComponent(const std::string& str);
    UnlessEscapedSubGrammarComponent();
    ~UnlessEscapedSubGrammarComponent();
    bool setSubGrammarComponent (SubGrammarComponent &componentToAdd);
    bool setEscapedSubGrammarComponent (SubGrammarComponent &componentToAdd);
};

class PopNameSubGrammarComponent : public SubGrammarComponent {
    public:
    PopNameSubGrammarComponent();
    ~PopNameSubGrammarComponent();
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class NoOpSubGrammarComponent : public SubGrammarComponent {
public:
    NoOpSubGrammarComponent();
    ~NoOpSubGrammarComponent();
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class ErrorSubGrammarComponent : public SubGrammarComponent {
private:
    std::string message;
public:
    ErrorSubGrammarComponent();
    ~ErrorSubGrammarComponent();
    ErrorSubGrammarComponent(std::string m);
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
    std::string toString() const;
};

class SubGrammar {
    friend class SubGrammarParser;
    std::vector<std::tuple<CharacterSet*, SubGrammarComponent *>> *components;
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
private:
    std::unordered_map<char32_t, SubGrammarComponent *> *componentsForCharacters;
    SubGrammarComponent *defaultComponent;
public:
    SubGrammarParser();
    SubGrammarParser(SubGrammar &forSubGrammar);
    ~SubGrammarParser();
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
};

class GrammarParser {
private:
    std::unordered_map<std::string, SubGrammarParser *> *subGrammarParsersByName;
public:
    GrammarParser();
    virtual ~GrammarParser();
    bool addSubGrammarParser (std::string name, SubGrammarParser *parser);
    ParserResult parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences);
};

#endif