#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <stack>
#include "parser.hpp"

std::string char32ToUtf8(char32_t ch) {
    std::u32string u32str(1, ch);  // wrap in u32string
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(u32str);
}

std::u32string UTF8toUTF32(const std::string& input) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    return converter.from_bytes(input);
}

std::string U32StringToString(const std::u32string& input) {
    // std::wstring_convert is deprecated in C++17, but still widely supported.
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    return converter.to_bytes(input);
}

std::string ParserNode::toString() const {
    return this->toString("");
}

std::string ParserNode::toString(std::string indentation) const {
    std::string parserNodeString = indentation + U32StringToString(*text) + "\n";
    for (ParserNode *childNode : *children) {
        parserNodeString += childNode->toString(indentation + "----");
    }
    return parserNodeString;
}

ParserNode::ParserNode(const std::string &n) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    text =  new std::u32string(converter.from_bytes(n));
    children = new std::vector<ParserNode *> ();
}

ParserNode::ParserNode(char32_t n) {
    text = new std::u32string(1, n);
    children = new std::vector<ParserNode *> ();
}

ParserNode::ParserNode() = default;

ParserNode::~ParserNode() { 
    //delete each parser node
    for (ParserNode *child : *children) {
        delete child;
    }
    //delete vector
    delete children;
    delete text;
}

ParserNode *ParserNode::getParent () {
    return parent;
}

ParserNode *ParserNode::createChild (const std::string &n) {
    ParserNode *nextChild = new ParserNode(n);
    nextChild->parent = this;
    children->push_back(nextChild);
    return nextChild;
}

ParserNode *ParserNode::createChild (char32_t n) {
    ParserNode *nextChild = new ParserNode(n);
    nextChild->parent = this;
    children->push_back(nextChild);
    return nextChild;
}

bool ParserNode::addCharacter (char32_t characterToAdd) {
    text->push_back(characterToAdd);
    return true;
}

ParserNode *ParserNode::lastChild () {
    return children->back();
}

ParserNode *ParserNode::getChild (int index) {
    if (index < 0) {
        index = children->size() + index;
    }
    return children->at(index);
}

std::u32string ParserNode::getValue () {
    return *new std::u32string(*text);
}

size_t ParserNode::size() {
    return children->size();
}

std::vector<char32_t> *CharacterSet::getCharacters () {
    return nullptr;
}

CharacterSet::~CharacterSet() = default;
CharacterSet::CharacterSet() = default;



StringCharacterSet::StringCharacterSet(const std::string& str) {
    characters = new std::vector<char32_t>;
    std::u32string u32str = UTF8toUTF32(str);
    for (char32_t ch : u32str) {
        characters->push_back(ch);
    }
}

StringCharacterSet::StringCharacterSet() = default;

StringCharacterSet::~StringCharacterSet() {
    delete characters;
}

std::vector<char32_t> *StringCharacterSet::getCharacters () {
    return characters;
}

ParserResult SubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    //add text to current node
    //empty and returning true proceeds through the string being parsed
    char32_t nextCharacter = characters[index];
    currentNode->addCharacter(nextCharacter);
    return { currentNode, "" };
}

std::string SubGrammarComponent::toString() const {
    return "subgrammar component";
}

SubGrammarComponent::SubGrammarComponent() = default;
SubGrammarComponent::~SubGrammarComponent() = default;

CompositeSubGrammarComponent::CompositeSubGrammarComponent () {
    subGrammarComponents = new std::vector<SubGrammarComponent *> ();
}

CompositeSubGrammarComponent::~CompositeSubGrammarComponent() {
    delete subGrammarComponents;
}

ParserResult CompositeSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    ParserResult result = {currentNode, ""};
    for (SubGrammarComponent *nextComponent : *subGrammarComponents) {
        result = nextComponent->parse(characters, index, currentNode, subGrammarReferences);
        if (result.node == nullptr) {
            break;
        }
        currentNode = result.node;
    }
    return result;
}

std::string CompositeSubGrammarComponent::toString() const {
    return "Composite subgrammar Component";
}

bool CompositeSubGrammarComponent::addSubGrammarComponent (SubGrammarComponent &componentToAdd) {
    subGrammarComponents->emplace_back(&componentToAdd);
    return true;
}

/*
* New sibling subgrammar component
*/

ParserResult NewSiblingSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    ParserResult result = {currentNode, ""};
    //create new sibling
    return { currentNode->getParent()->createChild(""), "" };
}

std::string NewSiblingSubGrammarComponent::toString() const {
    return "New sibling sub grammar component";
}

NewSiblingSubGrammarComponent::NewSiblingSubGrammarComponent() = default;
NewSiblingSubGrammarComponent::~NewSiblingSubGrammarComponent() = default;

/*
* No siblings subgrammar component
*/

ParserResult NoSiblingsSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    ParserResult result = {currentNode, ""};
    if (currentNode->getParent()->size() > 1) {
        std::string siblingName = U32StringToString(currentNode->getParent()->getChild(-2)->getValue());
        result = {nullptr, "Character cannot have siblings, but has sibling \"" + siblingName + "\""};
    }
    return result;
}

std::string NoSiblingsSubGrammarComponent::toString() const {
    return "No siblings sub grammar component";
}

NoSiblingsSubGrammarComponent::NoSiblingsSubGrammarComponent() = default;
NoSiblingsSubGrammarComponent::~NoSiblingsSubGrammarComponent() = default;

/*
* Push Subgrammar component
*/

ParserResult PushSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    //create sibling node with name "["
    //create child node of that node, a text node
    //return the text node

    //create sibling with the character invoking the push
    char32_t nextCharacter = characters[index];
    //the child of that sibling is now the current node
    currentNode = currentNode->createChild("");

    return {currentNode, ""};
}

std::string PushSubGrammarComponent::toString() const {
    return "Push subgrammar component";
}

PushSubGrammarComponent::PushSubGrammarComponent() = default;
PushSubGrammarComponent::~PushSubGrammarComponent() = default;

ParserResult PopSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    char32_t nextCharacter = characters[index];
    currentNode->getParent()->addCharacter(nextCharacter);
    currentNode = currentNode->getParent()->getParent()->lastChild();
    return {currentNode, ""};
}

std::string PopSubGrammarComponent::toString() const {
    return "Pop subgrammar component";
}

PopSubGrammarComponent::PopSubGrammarComponent() = default;
PopSubGrammarComponent::~PopSubGrammarComponent() = default;

/*
* Unless escaped subgrammar component
*/

ParserResult UnlessEscapedSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    //look back at last charaacter. If it's an escape, call the escaped subgrammar component. otherwise, call the regular one
    char32_t nextCharacter = characters[index];
    ParserResult result = {currentNode, ""};
    if (escapeCharacters->find(nextCharacter) != escapeCharacters->end()) {
        // Found it, so it has been escaped
        result = escapedComponent->parse(characters, index, currentNode, subGrammarReferences);
    }
    else {
        result = component->parse(characters, index, currentNode, subGrammarReferences);
    }
    return result;
}

std::string UnlessEscapedSubGrammarComponent::toString() const {
    return "Unless escaped subgrammar component";
}

bool UnlessEscapedSubGrammarComponent::setSubGrammarComponent (SubGrammarComponent &componentToAdd) {
    component = &componentToAdd;
    return true;
}

bool UnlessEscapedSubGrammarComponent::setEscapedSubGrammarComponent (SubGrammarComponent &componentToAdd) {
    escapedComponent = &componentToAdd;
    return true;
}

UnlessEscapedSubGrammarComponent::UnlessEscapedSubGrammarComponent(const std::string& str) {
    //push each character in string onto characters vector
    escapeCharacters = new std::unordered_set<char32_t> (); 
    for (char32_t characterInString : UTF8toUTF32(str)) {
        escapeCharacters->emplace(characterInString);
    }
}

UnlessEscapedSubGrammarComponent::UnlessEscapedSubGrammarComponent() = default;
UnlessEscapedSubGrammarComponent::~UnlessEscapedSubGrammarComponent() = default;

/*
* Push Name Sub Grammar Component
*/

PushNameSubGrammarComponent::PushNameSubGrammarComponent() = default;
PushNameSubGrammarComponent::~PushNameSubGrammarComponent() = default;

PushNameSubGrammarComponent::PushNameSubGrammarComponent(std::string n) {
    name = n;
}

ParserResult PushNameSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    subGrammarReferences->push(name);
    return {currentNode, ""};
}

std::string PushNameSubGrammarComponent::toString() const {
    return "Push name subgrammar component";
}

PopNameSubGrammarComponent::PopNameSubGrammarComponent() = default;
PopNameSubGrammarComponent::~PopNameSubGrammarComponent() = default;

ParserResult PopNameSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    subGrammarReferences->pop();
    return {currentNode, ""};
}

std::string PopNameSubGrammarComponent::toString() const {
    return "Pop name subgrammar component";
}

NoOpSubGrammarComponent::NoOpSubGrammarComponent() = default;
NoOpSubGrammarComponent::~NoOpSubGrammarComponent() = default;

ParserResult NoOpSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    return {currentNode, ""};
}

std::string NoOpSubGrammarComponent::toString() const {
    return "NoOp subgrammar component";
}


ErrorSubGrammarComponent::ErrorSubGrammarComponent() = default;
ErrorSubGrammarComponent::~ErrorSubGrammarComponent() = default;

ErrorSubGrammarComponent::ErrorSubGrammarComponent(std::string m) {
    message = m;
}

ParserResult ErrorSubGrammarComponent::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    return {nullptr, "Error: "+message };
}

std::string ErrorSubGrammarComponent::toString() const {
    return "Error subgrammar component";
}


SubGrammar::SubGrammar () {
    components = new std::vector<std::tuple<CharacterSet*, SubGrammarComponent *>> ();
}

SubGrammar::~SubGrammar () {
    delete components;
}

bool SubGrammar::addComponent(CharacterSet *onChars, SubGrammarComponent *component) {
    components->emplace_back(onChars, component);
    return true;
}

std::unordered_map<char32_t, SubGrammarComponent *> SubGrammar::asUnorderedMap () {
    std::unordered_map<char32_t, SubGrammarComponent *> charactersForComponents;
    for (std::tuple<CharacterSet*, SubGrammarComponent *>componentForCharacterSet : *components) {
        //each character set adds to unordered map multiple times
        std::vector<char32_t> *charactersForCharacterSet = std::get<0>(componentForCharacterSet)->getCharacters();
        for (char32_t characterForCharacterSet : *charactersForCharacterSet) {
            charactersForComponents.emplace(characterForCharacterSet, std::get<1>(componentForCharacterSet));
        }
    }

    for (const auto &pair: charactersForComponents) {
        const char32_t key = pair.first;
    }

    return charactersForComponents;
}

bool SubGrammar::setDefaultComponent (SubGrammarComponent &c) {
    defaultComponent = &c;
    return true;
}

SubGrammarParser::SubGrammarParser() {
    componentsForCharacters = new std::unordered_map<char32_t, SubGrammarComponent *>();
    defaultComponent = new SubGrammarComponent ();
}

SubGrammarParser::SubGrammarParser(SubGrammar &forSubGrammar) {
    componentsForCharacters = new std::unordered_map<char32_t, SubGrammarComponent *> (forSubGrammar.asUnorderedMap());
    //the "default default" behavior of the parser is the behavior of the subgrammarcomponent base class
    if (forSubGrammar.defaultComponent == nullptr) {
        defaultComponent = new SubGrammarComponent ();
    }
    else {
        defaultComponent = forSubGrammar.defaultComponent;
    }
}

SubGrammarParser::~SubGrammarParser() {
    delete componentsForCharacters;
}

ParserResult SubGrammarParser::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    SubGrammarComponent *subGrammarComponentForCharacter = defaultComponent;

    char32_t nextCharacter = characters[index];
    if (componentsForCharacters->find(nextCharacter) != componentsForCharacters->end()) {
        subGrammarComponentForCharacter = (*componentsForCharacters)[nextCharacter];
    }

    return subGrammarComponentForCharacter->parse(characters, index, currentNode, subGrammarReferences);
}

GrammarParser::GrammarParser() {
    subGrammarParsersByName = new std::unordered_map<std::string, SubGrammarParser *>();
}

GrammarParser::~GrammarParser() {
    delete subGrammarParsersByName;
}

bool GrammarParser::addSubGrammarParser (std::string name, SubGrammarParser *parser) {
    subGrammarParsersByName->emplace(name, parser);
    return true;
}

ParserResult GrammarParser::parse (char32_t *characters, int index, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
    ParserNode *nextNode = NULL;
    std::string subGrammarReference = subGrammarReferences->top();
    SubGrammarParser *subGrammarParser = (*subGrammarParsersByName)[subGrammarReference];
    ParserResult result;

    if (subGrammarParser == NULL) {
        result = { nullptr, "could not find subgrammar for key " + subGrammarReference + "\n" };
    }
    else {
        result = subGrammarParser->parse(characters, index, currentNode, subGrammarReferences);
    }

    return result;
}