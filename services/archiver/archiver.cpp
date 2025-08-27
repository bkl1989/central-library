#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <stack>
#include "parser.hpp"
#include "jsonParser.hpp"

//this file is not finalized. this won't be the format the consumer of the SDK will use.
int main() {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    std::u32string testParse = conv.from_bytes("these brackets create an object: [object]\"these brackets don't: [object]\"wow!");

    SubGrammar testSubGrammar, quoteSubGrammar;

    //Open bracket pushes
    StringCharacterSet openBracket("[");
    PushSubGrammarComponent pushComponent;
    testSubGrammar.addComponent(&openBracket, &pushComponent);

    //Close bracket pops
    StringCharacterSet closeBracket("]");
    PopSubGrammarComponent popComponent;
    testSubGrammar.addComponent(&closeBracket, &popComponent);

    // //quote pushes object and pushes the quote subGrammar
    StringCharacterSet quote("\"");
    CompositeSubGrammarComponent quoteCompositeComponent;
    quoteCompositeComponent.addSubGrammarComponent(pushComponent);
    PushNameSubGrammarComponent quoteBeginComponent("quote");
    quoteCompositeComponent.addSubGrammarComponent(quoteBeginComponent);
    testSubGrammar.addComponent(&quote, &quoteCompositeComponent);

    SubGrammarParser rootParser(testSubGrammar);
    GrammarParser parser;
    parser.addSubGrammarParser("root", &rootParser);

    //The only thing in quote syntax is an ending quote
    CompositeSubGrammarComponent quoteEndCompositeComponent;
    quoteEndCompositeComponent.addSubGrammarComponent(popComponent);
    PopNameSubGrammarComponent quoteEndComponent;
    quoteEndCompositeComponent.addSubGrammarComponent(quoteEndComponent);
    quoteSubGrammar.addComponent(&quote, &quoteEndCompositeComponent);
    SubGrammarParser quoteParser(quoteSubGrammar);
    parser.addSubGrammarParser("quote",&quoteParser);

    ParserNode rootNode("root");
    rootNode.createChild("");

    std::stack<std::string> subGrammarReferences;
    subGrammarReferences.push("root");

    ParserNode *currentNode = rootNode.lastChild();
    ParserResult result;

    for (char32_t nextCharacter : testParse) {
        result = parser.parse(nextCharacter, currentNode, &subGrammarReferences);
        currentNode = result.node;
        if (currentNode == nullptr) {
            std::cout << "Error parsing:" << result.error << "\n";
            break;
        }
    }
    
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> unConv;
    std::cout << "Objects from first parse:\n" + rootNode.toString() << "\n";

    std::u32string testJSONParse = conv.from_bytes(" \"json key\": \"json value\" ");
    std::stack<std::string> JSONSubGrammarReferences;
    JSONSubGrammarReferences.push("JSON");
    
    //i know... this isn't how it's going to work long-term
    JSONParser *testJSONParser = new JSONParser ();
    ParserNode JSONRootNode("{}");
    JSONRootNode.createChild("");
    currentNode = JSONRootNode.lastChild();

    for (char32_t nextCharacter : testJSONParse) {
        result = testJSONParser->parse(nextCharacter, currentNode, &JSONSubGrammarReferences);
        currentNode = result.node;
        if (currentNode == nullptr) {
            std::cout << "Error parsing JSON:" << result.error << "\n";
            break;
        }
    }

    std::cout << "JSON node structure: " << JSONRootNode.toString() << "\n";

    return 0;
}
