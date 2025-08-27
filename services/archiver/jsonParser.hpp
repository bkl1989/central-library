#ifndef CL_PARSER_JSON_PARSER_HPP
#define CL_PARSER_JSON_PARSER_HPP

#include "parser.hpp"
#include "jsonParser.hpp"

class JSONParser : public GrammarParser {
    public:
    JSONParser();
    ~JSONParser();
};

#endif