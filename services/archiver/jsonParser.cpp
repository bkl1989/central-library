#include "./parser.cpp"

//this file is not finalized. this won't be the format the consumer of the SDK will use.
GrammarParser constructJSONParser () {
    GrammarParser JSONParser;

    SubGrammar
        JSONObjectSubGrammar,
        JSONOValueSubGrammar,
        JSONArraySubGrammar,
        JSONStringSubGrammar;


    /*
    JSON Key subgrammar
    */
    StringCharacterSet whitespace(" ");
    //should noop be default behavior, rather than having to specify it?
    NoOpSubGrammarComponent noOpComponent;
    JSONObjectSubGrammar.addComponent(&whitespace, &noOpComponent);

    StringCharacterSet quote("\"");
    CompositeSubGrammarComponent quoteCompositeComponent;
    quoteCompositeComponent.addSubGrammarComponent(noOpComponent);
    PushNameSubGrammarComponent beginStringComponent("JSON.string");
    quoteCompositeComponent.addSubGrammarComponent(beginStringComponent);
    JSONObjectSubGrammar.addComponent(&quote, &quoteCompositeComponent);

    StringCharacterSet colon(":");
    CompositeSubGrammarComponent JSONValueCompositeComponent;
    JSONValueCompositeComponent.addSubGrammarComponent(noOpComponent);
    PushNameSubGrammarComponent beginValueComponent("JSON.value");
    JSONValueCompositeComponent.addSubGrammarComponent(beginValueComponent);
    JSONObjectSubGrammar.addComponent(&colon, &JSONValueCompositeComponent);

    ErrorSubGrammarComponent errorComponent("Invalid character in JSON Key expression");
    JSONObjectSubGrammar.setDefaultComponent(errorComponent);

    SubGrammarParser JSONObjectKeySubGrammarParser(JSONObjectSubGrammar);
    JSONParser.addSubGrammarParser("JSON.key", &JSONObjectKeySubGrammarParser);

    return JSONParser;
}