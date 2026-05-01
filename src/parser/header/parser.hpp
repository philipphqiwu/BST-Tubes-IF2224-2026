#ifndef PARSER_HPP
#define PARSER_HPP
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <initializer_list>

struct Token {
    std::string type;
    std::string value;
    int line;

    Token(std::string t = "", std::string v = "", int l = 0) : type(std::move(t)), value(std::move(v)), line(l) {}

    std::string display() const {
        if (value.empty()) return type;
        return type + "(" + value + ")";
    }
};

#endif