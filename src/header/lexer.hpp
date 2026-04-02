#ifndef LEXER_HPP
#define LEXER_HPP
#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <cctype>
#include <algorithm>

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
    sy_plus,
    sy_minus,
    sy_eql,
    sy_lss,
    sy_gt,
    sy_lpar,
    sy_colon,
};

extern std::unordered_map<std::string, std::string> keywordLookupTable;

void lexer(std::ifstream& input, std::ofstream& output);
bool isNumber(char c);
bool isAlphabet(char c);
bool isSymbol(char c);
bool isWhitespace(char c);
bool isJunk(char c);
void errorMsg(std::ofstream& output, int lineCnt, char c, bool& shouldExit);
void startBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit);
void identBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit);
std::string keywordLookup(std::string str);

#endif