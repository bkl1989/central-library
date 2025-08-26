#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <unordered_map>
#include <stack>
#include "./parser.cpp"

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
        if (currentNode == nullptr) {
            std::cout << "Error parsing:" << result.error << "\n";
            break;
        }
        else {
            currentNode = result.node;
        }
    }
    
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> unConv;
    std::cout << "Objects:\n" + rootNode.toString() << "\n";
    return 0;
}
