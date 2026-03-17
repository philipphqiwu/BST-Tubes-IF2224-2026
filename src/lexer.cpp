#include "lexer.hpp"

void lexer(std::ifstream& file){
    char c;

    while(file.get(c)){
        std::cout << c;
    }
}