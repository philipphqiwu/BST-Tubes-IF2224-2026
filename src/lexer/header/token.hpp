#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

struct Token {
    std::string type;
    std::string value;
    int line;

    Token(std::string t = "", std::string v = "", int l = 0) : type(std::move(t)), value(std::move(v)), line(l) {}

    std::string display() const {
        if (value.empty()) return type;
        return type + " (" + value + ")";
    }
};

#endif