#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <stack>

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

class ParserNode {
private:
    std::u32string *text;
    std::vector<ParserNode *> *children;
    ParserNode *parent = NULL;
public:
    std::string toString() const {
        return this->toString("");
    }

    std::string toString(std::string indentation) const {
        std::string parserNodeString = indentation + U32StringToString(*text) + "\n";
        for (ParserNode *childNode : *children) {
            parserNodeString += childNode->toString(indentation + "----");
        }
        return parserNodeString;
    }

    ParserNode(const std::string &n) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        text =  new std::u32string(converter.from_bytes(n));
        children = new std::vector<ParserNode *> ();
    }

    ParserNode(char32_t n) {
        text = new std::u32string(1, n);
        children = new std::vector<ParserNode *> ();
    }

    ~ParserNode() {
        //delete each parser node
        for (ParserNode *child : *children) {
            delete child;
        }
        //delete vector
        delete children;
        delete text;
    }

    ParserNode *getParent () {
        return parent;
    }

    ParserNode *createChild (const std::string &n) {
        ParserNode *nextChild = new ParserNode(n);
        nextChild->parent = this;
        children->push_back(nextChild);
        return nextChild;
    }

    ParserNode *createChild (char32_t n) {
        ParserNode *nextChild = new ParserNode(n);
        nextChild->parent = this;
        children->push_back(nextChild);
        return nextChild;
    }

    bool addCharacter (char32_t characterToAdd) {
        text->push_back(characterToAdd);
        return true;
    }

    ParserNode *lastChild () {
        return children->back();
    }

    std::u32string getValue () {
        return *new std::u32string(*text);
    }
};

struct ParserResult {
    ParserNode *node;
    std::string error;
};

class CharacterSet {
public:
    virtual std::vector<char32_t> *getCharacters () {
        return nullptr;
    };

    virtual ~CharacterSet() = default;
};

class StringCharacterSet : public CharacterSet {
private:
    std::vector<char32_t> *characters;
public:
    StringCharacterSet(const std::string& str) {
        characters = new std::vector<char32_t>;
        std::u32string u32str = UTF8toUTF32(str);
        for (char32_t ch : u32str) {
            characters->push_back(ch);
        }
    }

    ~StringCharacterSet() {
        delete characters;
    }

    std::vector<char32_t> *getCharacters () {
        return characters;
    }
};

class SubGrammarComponent {
    public:
    virtual ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        //add text to current node
        //empty and returning true proceeds through the string being parsed
        currentNode->addCharacter(nextCharacter);
        return { currentNode, "" };
    }

    virtual std::string toString() const {
        return "subgrammar component";
    }
    
    virtual ~SubGrammarComponent() = default;
};

class CompositeSubGrammarComponent : public SubGrammarComponent {
    std::vector<SubGrammarComponent *> *subGrammarComponents;

    public:
    CompositeSubGrammarComponent () {
        subGrammarComponents = new std::vector<SubGrammarComponent *> ();
    }

    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        ParserResult result = {currentNode, ""};
        for (SubGrammarComponent *nextComponent : *subGrammarComponents) {
            result = nextComponent->parse(nextCharacter, currentNode, subGrammarReferences);
            if (result.node == nullptr) {
                break;
            }
            currentNode = result.node;
        }
        return result;
    }

    std::string toString() const {
        return "Composite subgrammar Component";
    }

    bool addSubGrammarComponent (SubGrammarComponent &componentToAdd) {
        subGrammarComponents->emplace_back(&componentToAdd);
        return true;
    }
};

class PushSubGrammarComponent : public SubGrammarComponent {
    public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        //create sibling node with name "["
        //create child node of that node, a text node
        //return the text node

        //create sibling with the character invoking the push
        currentNode = currentNode->getParent()->createChild(nextCharacter);
        //the child of that sibling is now the current node
        currentNode = currentNode->createChild("");

        return {currentNode, ""};
    }

    std::string toString() const {
        return "Push subgrammar component";
    }
};

class PopSubGrammarComponent : public SubGrammarComponent {
    public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        currentNode->getParent()->addCharacter(nextCharacter);
        currentNode = currentNode->getParent()->getParent()->createChild("");
        return {currentNode, ""};
    }

    std::string toString() const {
        return "Pop subgrammar component";
    }
};

class PushNameSubGrammarComponent : public SubGrammarComponent {
    std::string name;

    public:
    PushNameSubGrammarComponent(std::string n) {
        name = n;
    }

    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        subGrammarReferences->push(name);
        return {currentNode, ""};
    }

    std::string toString() const {
        return "Push name subgrammar component";
    }
};

class PopNameSubGrammarComponent : public SubGrammarComponent {
    public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        subGrammarReferences->pop();
        return {currentNode, ""};
    }

    std::string toString() const {
        return "Pop name subgrammar component";
    }
};

class NoOpSubGrammarComponent : public SubGrammarComponent {
public:
    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        return {currentNode, ""};
    }

    std::string toString() const {
        return "NoOp subgrammar component";
    }
};

class ErrorSubGrammarComponent : public SubGrammarComponent {
private:
    std::string message;
public:
    ErrorSubGrammarComponent(std::string m) {
        message = m;
    }

    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        return {nullptr, "Error: "+message };
    }

    std::string toString() const {
        return "Error subgrammar component";
    }
};

class SubGrammar {
    friend class SubGrammarParser;
private:
    std::vector<std::tuple<CharacterSet*, SubGrammarComponent *>> *components;
protected:
    SubGrammarComponent *defaultComponent = nullptr;
public:
    SubGrammar () {
        components = new std::vector<std::tuple<CharacterSet*, SubGrammarComponent *>> ();
    }

    ~SubGrammar () {
        delete components;
    }

    bool addComponent(CharacterSet *onChars, SubGrammarComponent *component) {
        components->emplace_back(onChars, component);
        return true;
    }

    std::unordered_map<char32_t, SubGrammarComponent *> asUnorderedMap () {
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

    bool setDefaultComponent (SubGrammarComponent &c) {
        defaultComponent = &c;
        return true;
    }
};

// Formats subGrammar data so that it can be most efficiently used for parsing
class SubGrammarParser {
private:
    std::unordered_map<char32_t, SubGrammarComponent *> *componentsForCharacters;
    SubGrammarComponent *defaultComponent;
public:
    SubGrammarParser(SubGrammar &forSubGrammar) {
        componentsForCharacters = new std::unordered_map<char32_t, SubGrammarComponent *> (forSubGrammar.asUnorderedMap());
        //the "default default" behavior of the parser is the behavior of the subgrammarcomponent base class
        if (forSubGrammar.defaultComponent == nullptr) {
            defaultComponent = new SubGrammarComponent ();
        }
        else {
            defaultComponent = forSubGrammar.defaultComponent;
        }
    }

    ~SubGrammarParser() {
        delete componentsForCharacters;
        delete defaultComponent;
    }

    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        SubGrammarComponent *subGrammarComponentForCharacter = defaultComponent;
        if (componentsForCharacters->find(nextCharacter) != componentsForCharacters->end()) {
            subGrammarComponentForCharacter = (*componentsForCharacters)[nextCharacter];
        }

        return subGrammarComponentForCharacter->parse(nextCharacter, currentNode, subGrammarReferences);
    }
};

class GrammarParser {
private:
    std::unordered_map<std::string, SubGrammarParser *> *subGrammarParsersByName;
public:
    GrammarParser() {
        subGrammarParsersByName = new std::unordered_map<std::string, SubGrammarParser *>();
    }

    ~GrammarParser() {
        delete subGrammarParsersByName;
    }

    bool addSubGrammarParser (std::string name, SubGrammarParser *parser) {
        subGrammarParsersByName->emplace(name, parser);
        return true;
    }

    ParserResult parse (char32_t nextCharacter, ParserNode *currentNode, std::stack<std::string> *subGrammarReferences) {
        ParserNode *nextNode = NULL;
        std::string subGrammarReference = subGrammarReferences->top();
        SubGrammarParser *subGrammarParser = (*subGrammarParsersByName)[subGrammarReference];
        ParserResult result;

        if (subGrammarParser == NULL) {
            result = { nullptr, "could not find sub grammar for key " + subGrammarReference + "\n" };
        }
        else {
            result = subGrammarParser->parse(nextCharacter, currentNode, subGrammarReferences);
        }
    
        return result;
    }
};