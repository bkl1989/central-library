#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <unordered_map>

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
            parserNodeString += childNode->toString(indentation + "  ");
        }
        return parserNodeString;
    }

    ParserNode(const std::string &n) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        text = new std::u32string(converter.from_bytes(n));
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

class CharacterSet {
private:
    std::vector<char32_t> *characters;
public:
    CharacterSet(const std::string& str) {
        characters = new std::vector<char32_t>;
        std::u32string u32str = UTF8toUTF32(str);
        for (char32_t ch : u32str) {
            characters->push_back(ch);
        }
    }

    ~CharacterSet() {
        delete characters;
    }

    std::vector<char32_t> getCharacters () {
        return *characters;
    }
};

class GrammarComponent {
    public:
    virtual ParserNode *parse (char32_t nextCharacter, ParserNode *currentNode) {
        //add text to current node
        //empty and returning true proceeds through the string being parsed
        currentNode->addCharacter(nextCharacter);
        return currentNode;
    }

    virtual std::string toString() const {
        return "Grammar component";
    }
};

class PushGrammarComponent : public GrammarComponent {
    public:
    ParserNode *parse (char32_t nextCharacter, ParserNode *currentNode) {
        //create sibling node with name "["
        //create child node of that node, a text node
        //return the text node

        //create sibling with the character invoking the push
        currentNode = currentNode->getParent()->createChild(nextCharacter);
        //the child of that sibling is now the current node
        currentNode = currentNode->createChild("");

        return currentNode;
    }

    std::string toString() const {
        return "Push grammar component";
    }
};

class PopGrammarComponent : public GrammarComponent {
    public:
    ParserNode *parse (char32_t nextCharacter, ParserNode *currentNode) {
        currentNode->getParent()->addCharacter(nextCharacter);
        currentNode = currentNode->getParent()->getParent()->createChild("");
        return currentNode;
    }

    std::string toString() const {
        return "Pop grammar component";
    }
};

class Grammar {
private:
    std::vector<std::tuple<CharacterSet*, GrammarComponent *>> *components;
public:
    Grammar () {
        components = new std::vector<std::tuple<CharacterSet*, GrammarComponent *>> ();
    }

    ~Grammar () {
        delete components;
    }

    bool addComponent(CharacterSet *onChars, GrammarComponent *component) {
        components->emplace_back(onChars, component);
        return true;
    }

    std::unordered_map<char32_t, GrammarComponent *> asUnorderedMap () {
        std::unordered_map<char32_t, GrammarComponent *> charactersForComponents;
        for (std::tuple<CharacterSet*, GrammarComponent *>componentForCharacterSet : *components) {
            //each character set adds to unordered map multiple times
            std::vector<char32_t> charactersForCharacterSet = std::get<0>(componentForCharacterSet)->getCharacters();
            for (char32_t characterForCharacterSet :  charactersForCharacterSet) {
                charactersForComponents.emplace(characterForCharacterSet, std::get<1>(componentForCharacterSet));
            }
        }

        for (const auto &pair: charactersForComponents) {
            const char32_t key = pair.first;
        }

        return charactersForComponents;
    }
};

// Formats grammar data so that it can be most efficiently used for parsing
class GrammarParser {
private:
    std::unordered_map<char32_t, GrammarComponent *> *componentsForCharacters;
    GrammarComponent *defaultComponent;
public:
    GrammarParser(Grammar &forGrammar) {
        componentsForCharacters = new std::unordered_map<char32_t, GrammarComponent *> (forGrammar.asUnorderedMap());
        defaultComponent = new GrammarComponent ();
    }

    ~GrammarParser() {
        delete componentsForCharacters;
        delete defaultComponent;
    }

    ParserNode *parse (char32_t nextCharacter, ParserNode *currentNode) {
        GrammarComponent *grammarComponentForCharacter = defaultComponent;
        if (componentsForCharacters->find(nextCharacter) != componentsForCharacters->end()) {
            grammarComponentForCharacter = (*componentsForCharacters)[nextCharacter];
        }

        ParserNode *result = grammarComponentForCharacter->parse(nextCharacter, currentNode);
        return result;
    }
};

int main() {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::u32string testParse = conv.from_bytes("outer[[[inner]]]outer2");

    Grammar testGrammar;

    //Open bracket pushes
    CharacterSet openBracket("[");
    PushGrammarComponent pushComponent;
    testGrammar.addComponent(&openBracket, &pushComponent);

    //Close bracket pops
    CharacterSet closeBracket("]");
    PopGrammarComponent popComponent;
    testGrammar.addComponent(&closeBracket, &popComponent);
                 
    GrammarParser parser(testGrammar);

    ParserNode rootNode("root");
    rootNode.createChild("");

    ParserNode *currentNode = rootNode.lastChild();
    for (char32_t nextCharacter : testParse) {
        currentNode = parser.parse(nextCharacter, currentNode);
    }
    
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> unConv;
    std::cout << "Objects:\n" + rootNode.toString() << "\n";
    return 0;
}
