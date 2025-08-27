#include "jsonParser.hpp"
#include "parser.hpp"

//this file is not finalized. this won't be the format the consumer of the SDK will use.
JSONParser::JSONParser () {
    SubGrammar
        *JSONObjectSubGrammar = new SubGrammar(),
        *JSONOValueSubGrammar = new SubGrammar(),
        *JSONArraySubGrammar = new SubGrammar(),
        *JSONStringSubGrammar = new SubGrammar();


    /*
    JSON Key subgrammar
    */
    StringCharacterSet *whitespace = new StringCharacterSet(" \t\n\r");
    //should noop be default behavior, rather than having to specify it?
    NoOpSubGrammarComponent *noOpComponent = new NoOpSubGrammarComponent();
    JSONObjectSubGrammar->addComponent(whitespace, noOpComponent);

    StringCharacterSet *quote = new StringCharacterSet("\"");
    CompositeSubGrammarComponent *quoteCompositeComponent = new CompositeSubGrammarComponent();
    quoteCompositeComponent->addSubGrammarComponent(*noOpComponent);
    PushNameSubGrammarComponent *beginStringComponent = new PushNameSubGrammarComponent("JSON.string");
    quoteCompositeComponent->addSubGrammarComponent(*beginStringComponent);
    JSONObjectSubGrammar->addComponent(quote, quoteCompositeComponent);

    StringCharacterSet *colon = new StringCharacterSet(":");
    CompositeSubGrammarComponent *JSONValueCompositeComponent = new CompositeSubGrammarComponent();
    JSONValueCompositeComponent->addSubGrammarComponent(*noOpComponent);
    PushNameSubGrammarComponent *beginValueComponent = new PushNameSubGrammarComponent("JSON.value");
    JSONValueCompositeComponent->addSubGrammarComponent(*beginValueComponent);
    JSONObjectSubGrammar->addComponent(colon, JSONValueCompositeComponent);

    /*
    JSON String subgrammar
    */

    //quotes (unless escaped with a preceding backslash)

    //Pops name and 
    CompositeSubGrammarComponent *stringEndCompositeComponent = new CompositeSubGrammarComponent();
    PopNameSubGrammarComponent *endStringComponent = new PopNameSubGrammarComponent();
    stringEndCompositeComponent->addSubGrammarComponent(*endStringComponent);

    SubGrammarComponent *writeComponent = new SubGrammarComponent();
    UnlessEscapedSubGrammarComponent *quoteUnlessEscapedComponent = new UnlessEscapedSubGrammarComponent("\"");
    quoteUnlessEscapedComponent->addEscapeSubGrammarComponent(*writeComponent);
    quoteUnlessEscapedComponent->addSubGrammarComponent(*stringEndCompositeComponent);

    //unescaped newlines give an error
    StringCharacterSet *

    /*
    JSON parser defaults to an error. It's defined by inclusion
    */
    ErrorSubGrammarComponent *errorComponent = new ErrorSubGrammarComponent("Invalid character in JSON Key expression");
    JSONObjectSubGrammar->setDefaultComponent(*errorComponent);

    SubGrammarParser *JSONObjectSubGrammarParser = new SubGrammarParser(*JSONObjectSubGrammar);
    addSubGrammarParser("JSON", JSONObjectSubGrammarParser);
}

JSONParser::~JSONParser() {
    //Any variable declared inside the above constructor should be moved to the header and destroyed here
    //But I have to figure out if I'm settling for this memory management system
}