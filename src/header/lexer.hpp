#ifndef LEXER_HPP
#define LEXER_HPP

#include <iostream>
#include <string>
#include <fstream>

enum State {
    state1,
    state2,
};

void lexer(std::ifstream& file);

#endif