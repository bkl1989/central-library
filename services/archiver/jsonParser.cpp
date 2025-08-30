#include "jsonParser.hpp"
#include "parser.hpp"

//this file is not finalized. this won't be the format the consumer of the SDK will use.
JSONParser::JSONParser () {
    SubGrammar
        *JSONObjectSubGrammar = new SubGrammar(),
        *JSONValueSubGrammar = new SubGrammar(),
        *JSONArraySubGrammar = new SubGrammar(),
        *JSONStringSubGrammar = new SubGrammar();

    PushSubGrammarComponent *pushComponent = new PushSubGrammarComponent ();

    /*
    JSON Key subgrammar
    */
    StringCharacterSet *whitespace = new StringCharacterSet(" \t\n\r");
    //should noop be default behavior, rather than having to specify it?
    NoOpSubGrammarComponent *noOpComponent = new NoOpSubGrammarComponent();
    JSONObjectSubGrammar->addComponent(whitespace, noOpComponent);

    StringCharacterSet *quote = new StringCharacterSet("\"");
    CompositeSubGrammarComponent *quoteCompositeComponent = new CompositeSubGrammarComponent();
    PushNameSubGrammarComponent *beginStringComponent = new PushNameSubGrammarComponent("JSON.string");
    quoteCompositeComponent->addSubGrammarComponent(*beginStringComponent);
    NoEmptySiblingsSubGrammarComponent *noEmptySiblingsComponent = new NoEmptySiblingsSubGrammarComponent();
    quoteCompositeComponent->addSubGrammarComponent(*noEmptySiblingsComponent);
    JSONObjectSubGrammar->addComponent(quote, quoteCompositeComponent);

    StringCharacterSet *colon = new StringCharacterSet(":");
    CompositeSubGrammarComponent *JSONValueCompositeComponent = new CompositeSubGrammarComponent();
    JSONValueCompositeComponent->addSubGrammarComponent(*noOpComponent);
    PushNameSubGrammarComponent *beginValueComponent = new PushNameSubGrammarComponent("JSON.value");
    JSONValueCompositeComponent->addSubGrammarComponent(*beginValueComponent);
    JSONValueCompositeComponent->addSubGrammarComponent(*pushComponent);
    JSONObjectSubGrammar->addComponent(colon, JSONValueCompositeComponent);

    /*
    JSON String subgrammar
    */

    //quotes (unless escaped with a preceding backslash)

    //Pops name and 

    CompositeSubGrammarComponent *stringEndCompositeComponent = new CompositeSubGrammarComponent();
    PopNameSubGrammarComponent *endStringComponent = new PopNameSubGrammarComponent();
    stringEndCompositeComponent->addSubGrammarComponent(*endStringComponent);
    // NewSiblingSubGrammarComponent *newSiblingComponent = new NewSiblingSubGrammarComponent ();
    // stringEndCompositeComponent->addSubGrammarComponent(*newSiblingComponent);

    SubGrammarComponent *writeComponent = new SubGrammarComponent();
    UnlessEscapedSubGrammarComponent *quoteUnlessEscapedComponent = new UnlessEscapedSubGrammarComponent("\\");
    quoteUnlessEscapedComponent->setEscapedSubGrammarComponent(*writeComponent);
    quoteUnlessEscapedComponent->setSubGrammarComponent(*stringEndCompositeComponent);
    JSONStringSubGrammar->addComponent(quote, quoteUnlessEscapedComponent);

    //unescaped newlines give an error
    StringCharacterSet *newline = new StringCharacterSet("\r\n");
    ErrorSubGrammarComponent *newlineInJSONStringError = new ErrorSubGrammarComponent("Newline found in JSON string");
    JSONStringSubGrammar->addComponent(newline, newlineInJSONStringError);

    SubGrammarParser *JSONStringSubGrammarParser = new SubGrammarParser(*JSONStringSubGrammar);
    addSubGrammarParser("JSON.string", JSONStringSubGrammarParser);

    /*
    * JSON Value subgrammar
    */

    //Open bracket { pushes a json object grammar
    StringCharacterSet *openCurlyBrace = new StringCharacterSet("{");
    CompositeSubGrammarComponent *JSONObjectGrammarComposite = new CompositeSubGrammarComponent();
    PushNameSubGrammarComponent *PushJSONGrammarComponent = new PushNameSubGrammarComponent("JSON");
    JSONObjectGrammarComposite->addSubGrammarComponent(*PushJSONGrammarComponent);
    JSONObjectGrammarComposite->addSubGrammarComponent(*pushComponent);
    JSONValueSubGrammar->addComponent(openCurlyBrace, JSONObjectGrammarComposite);
    //Open bracket [ pushes a json array grammar
    StringCharacterSet *openBracket = new StringCharacterSet("[");
    CompositeSubGrammarComponent *JSONArrayGrammarComposite = new CompositeSubGrammarComponent();
    PushNameSubGrammarComponent *pushJSONArrayGrammarComponent = new PushNameSubGrammarComponent("JSON.array");
    JSONArrayGrammarComposite->addSubGrammarComponent(*pushJSONArrayGrammarComponent);
    JSONArrayGrammarComposite->addSubGrammarComponent(*pushComponent);
    JSONValueSubGrammar->addComponent(openBracket, JSONArrayGrammarComposite);

    //Open quote pushes a json string
        //StringCharacterSet *quote defined
    JSONValueSubGrammar->addComponent(quote, quoteCompositeComponent);
    //Any number starts a number grammar
    StringCharacterSet *numberCharacterSet = new StringCharacterSet("0123456789"); 
    PushNameSubGrammarComponent *PushJSONNumberGrammarComponent = new PushNameSubGrammarComponent("JSON.number");
    JSONValueSubGrammar->addComponent(numberCharacterSet, PushJSONNumberGrammarComponent);

    //Comma pops object and grammar
    StringCharacterSet *comma = new StringCharacterSet(",");
    CompositeSubGrammarComponent *JSONValueTerminatorComposite = new CompositeSubGrammarComponent ();
    //errors out if it occurs before a value is present
    //so if the current child is blank
    NoEmptyValueSubGrammarComponent *noEmptyValueComponent = new NoEmptyValueSubGrammarComponent();
    JSONValueTerminatorComposite->addSubGrammarComponent(*noEmptyValueComponent);
    JSONValueSubGrammar->addComponent(comma, JSONValueTerminatorComposite);
    //pops object
    PopSubGrammarComponent *popComponent = new PopSubGrammarComponent();
    JSONValueTerminatorComposite->addSubGrammarComponent(*popComponent);
    //pops name
    PopNameSubGrammarComponent *popNameComponent = new PopNameSubGrammarComponent();
    JSONValueTerminatorComposite->addSubGrammarComponent(*popNameComponent);
    //creates new sibling
    NewSiblingSubGrammarComponent *newSiblingComponent = new NewSiblingSubGrammarComponent();
    JSONValueTerminatorComposite->addSubGrammarComponent(*newSiblingComponent);

    SubGrammarParser *JSONValueSubGrammarParser = new SubGrammarParser (*JSONValueSubGrammar);
    addSubGrammarParser("JSON.value", JSONValueSubGrammarParser);
    //Whitespace is ignored
        //StringCharacterSet *whitespace defined
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