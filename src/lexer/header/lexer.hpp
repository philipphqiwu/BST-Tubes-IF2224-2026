#ifndef LEXER_HPP
#define LEXER_HPP
#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cctype>
#include <algorithm>

#include "token.hpp"

enum State {
    Start,
    CommentCurly,
    CommentStar,
    CommentStarClose,
    Number,
    RealBegin,
    Real,
    CharBegin,
    Char,
    String,
    ident,
    sy_minus,
    sy_eql,
    sy_lss,
    sy_gt,
    sy_lpar,
    sy_colon,
    sy_period,
    Unknown
};

extern std::unordered_map<std::string, std::string> keywordLookupTable;

std::vector<Token> lexer(std::ifstream& input, std::ofstream& output);
bool isNumber(char c);
bool isAlphabet(char c);
bool isSymbol(char c);
bool isWhitespace(char c);
bool isJunk(char c);
void startBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit, std::vector<Token>& tokens);
void identBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit, std::vector<Token>& tokens);
std::string keywordLookup(std::string str);

#endif