#include "header/lexer.hpp"
#include <vector>

std::vector<Token> lexer(std::ifstream& input, std::ofstream& output){
    int state = Start;
    std::vector<Token> tokens;
    char c;
    bool shouldExit = false;
    bool stringHadEscape = false;
    (void)stringHadEscape;

    std::string str = "";
    int lineCnt = 1;

    while(input.get(c)){
        if(shouldExit) return tokens;
        if(c == '\n'){
            lineCnt++;
        }
        switch(state){
            // Start/empty
            case Start:
                startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                break;

            // Komen
            case CommentCurly:
                if(c == '}'){
                    tokens.emplace_back("comment", "", lineCnt);
                    output << "comment\n";
                    state = Start;
                } 
                break;
            
            case CommentStar:
                if(c == '*'){
                    state = CommentStarClose;
                }
                break;

            case CommentStarClose:
                if(c == ')'){
                    tokens.emplace_back("comment", "", lineCnt);
                    output << "comment\n";
                    state = Start;
                } else if(c != '*'){
                    state = CommentStar;
                }
                break;

            // Number
            case Number:
                if(!isNumber(c) && c != '.'){
                    tokens.emplace_back("intcon", str, lineCnt);
                    output << "intcon (" << str << ")\n";
                    str = "";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                } else if(isNumber(c)){
                    str += c;
                } else if(c == '.'){
                    if (input.peek() == '.') {
                        tokens.emplace_back("intcon", str, lineCnt);
                        output << "intcon (" << str << ")\n";
                        
                        tokens.emplace_back("period", "", lineCnt);
                        output << "period\n";
                        
                        input.get(c); 
                        tokens.emplace_back("period", "", lineCnt);
                        output << "period\n";
                        
                        str = "";
                        state = Start;
                    } else {
                        str += c;
                        state = RealBegin;
                    }
                }
                break;

            case RealBegin:
                if(!isNumber(c)){
                    state = Unknown;
                    str += c;
                } else if(isNumber(c)){
                    str += c;
                    state = Real;
                }
                break;

            case Real:
                if(!isNumber(c)){
                    tokens.emplace_back("realcon", str, lineCnt);
                    output << "realcon (" << str << ")\n";
                    str = "";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                } else if(isNumber(c)){
                    str += c;
                }
                break;

            // Char / string
            case CharBegin:
                if (c == '\'') {
                    if (input.peek() == '\''){
                        input.get(c);
                        str += '\'';
                        stringHadEscape = true;
                        state = String;
                    } else {
                        tokens.emplace_back("string", "'" + str + "'", lineCnt);
                        output << "string ('" << str << "')\n";
                        str = "";
                        stringHadEscape = false;
                        state = Start;
                    }
                } else {
                    str += c;
                    state = Char;
                }
                break;
            case Char:
                if(c == '\''){
                    tokens.emplace_back("charcon", str, lineCnt);
                    output << "charcon (" << str << ")\n";
                    str = "";
                    state = Start;
                } else {
                    str += c;
                    state = String;
                }
                break;
            case String:
                if (c == '\n'){
                    tokens.emplace_back("unknown", str, lineCnt);
                    output << "unknown (" << str << ")\n";
                    str = "";
                    state = Start;
                }
                if (c == '\'') {
                    if (input.peek() == '\''){
                        input.get(c);
                        str += '\'';
                        stringHadEscape = true;
                    } else{
                        tokens.emplace_back("string", "'" + str + "'", lineCnt);
                        output << "string ('" << str << "')\n";
                        str = "";
                        stringHadEscape = false;
                        state = Start;
                    }
                } else {
                    str += c;
                }
                break;

            // Keyword / ident
            case ident:
                identBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                break;

            // Simbol / operator
            case sy_minus:
                if(isNumber(c)){
                    str = "-";
                    str += c;
                    state = Number;
                } else{
                    tokens.emplace_back("minus", "", lineCnt);
                    output << "minus\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                }
                break;
            case sy_eql:
                if(c == '='){
                    tokens.emplace_back("eql", "", lineCnt);
                    output << "eql\n";
                    state = Start;
                } else{
                    str = "=";
                    if (!isWhitespace(c)) {
                        str += c;
                        state = Unknown;
                    } else {
                        tokens.emplace_back("unknown", "=", lineCnt);
                        output << "unknown (=)\n";
                        state = Start;
                    }
                }
                break;
            case sy_lss:
                if(c == '>'){
                    tokens.emplace_back("neq", "", lineCnt);
                    output << "neq\n";
                    state = Start;
                } else if(c == '='){
                    tokens.emplace_back("leq", "", lineCnt);
                    output << "leq\n";
                    state = Start;
                } else if(isWhitespace(c)){
                    tokens.emplace_back("lss", "", lineCnt);
                    output << "lss\n";
                    state = Start;
                } else{
                    tokens.emplace_back("lss", "", lineCnt);
                    output << "lss\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                }
                break;
            case sy_gt:
                if(c == '='){
                    tokens.emplace_back("geq", "", lineCnt);
                    output << "geq\n";
                    state = Start;
                } else if(isWhitespace(c)){
                    tokens.emplace_back("gtr", "", lineCnt);
                    output << "gtr\n";
                    state = Start;
                } else{
                    tokens.emplace_back("gtr", "", lineCnt);
                    output << "gtr\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                }
                break;
            case sy_lpar:
                if(c == '*'){
                    state = CommentStar;
                } else{
                    tokens.emplace_back("lparent", "", lineCnt);
                    output << "lparent\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                }
                break;
            case sy_colon:
                if(c == '='){
                    tokens.emplace_back("becomes", "", lineCnt);
                    output << "becomes\n";
                    state = Start;
                } else{
                    tokens.emplace_back("colon", "", lineCnt);
                    output << "colon\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                }
                break;
            case sy_period: 
                if (isWhitespace(c) || isAlphabet(c) || c == ';' || c == ',' || c == '(' || c == ')' || c == '[' || c == ']' || c == '.') {
                    tokens.emplace_back("period", "", lineCnt);
                    output << "period\n";
                    str = "";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                } else {
                    state = Unknown;
                    str += c;
                }
                break;
            case Unknown:
                if (isWhitespace(c) || c == ';' || c == ',' || c == '(' || c == ')' || c == '[' || c == ']') {
                    tokens.emplace_back("unknown", str, lineCnt);
                    output << "unknown (" << str << ")\n";
                    str = "";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
                } else {
                    str += c;
                }
                break;
        }
    }

    // Process at EOF
    if(input.eof() && state != Start){
        if(state == CommentCurly || state == CommentStar || state == CommentStarClose){
            tokens.emplace_back("unknown", "unclosed_comment", lineCnt);
            output << "unknown (unclosed_comment)\n";
        } else if(state == RealBegin || state == CharBegin || state == Char || state == String || state == Unknown){
            tokens.emplace_back("unknown", str, lineCnt);
            output << "unknown (" << str << ")\n";
        } else if(state == sy_eql){
            tokens.emplace_back("unknown", "=", lineCnt);
            output << "unknown (=)\n";
        } else if(state == Number){
            tokens.emplace_back("intcon", str, lineCnt);
            output << "intcon (" << str << ")\n";
        } else if(state == Real){
            tokens.emplace_back("realcon", str, lineCnt);
            output << "realcon (" << str << ")\n";
        } else if(state == sy_minus){
            tokens.emplace_back("minus", "", lineCnt);
            output << "minus\n";
        } else if(state == sy_lss){
            tokens.emplace_back("lss", "", lineCnt);
            output << "lss\n";
        } else if(state == sy_gt){
            tokens.emplace_back("gtr", "", lineCnt);
            output << "gtr\n";
        } else if(state == sy_colon){
            tokens.emplace_back("colon", "", lineCnt);
            output << "colon\n";
        } else if(state == sy_lpar){
            tokens.emplace_back("lparent", "", lineCnt);
            output << "lparent\n";
        } else if(state == sy_period){ // TAMBAHAN UNTUK TITIK DI AKHIR FILE
            tokens.emplace_back("period", "", lineCnt);
            output << "period\n";
        } else {
            std::string identOrKw = keywordLookup(str);
            if(identOrKw == "ident"){
                tokens.emplace_back("ident", str, lineCnt);
                output << "ident (" << str << ")\n";
            } else{
                tokens.emplace_back(identOrKw, "", lineCnt);
                output << identOrKw << "\n";
            }
        }
    }

    return tokens;
}

bool isNumber(char c){
    return (c >= '0' && c <= '9');
}
bool isAlphabet(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool isSymbol(char c){
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == ')' || c == '[' || c == ']' || c == ',' || c == ';' || c == '.');
}
bool isJunk(char c){
    return !isNumber(c) && !isAlphabet(c) && !isSymbol(c);
}
bool isWhitespace(char c){
    return (c == '\n' || c == '\r' || c == ' ');
}
void startBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit, std::vector<Token>& tokens){
    switch(c){
        case '{':
            state = CommentCurly;
            break;
        case '\'':
            state = CharBegin;
            break;
        case '+':
            tokens.emplace_back("plus", "", lineCnt);
            output << "plus\n";
            break;
        case '-':
            state = sy_minus;
            break;
        case '*':
            tokens.emplace_back("times", "", lineCnt);
            output << "times\n";
            break;
        case '/':
            tokens.emplace_back("rdiv", "", lineCnt);
            output << "rdiv\n";
            break;
        case '=':
            tokens.emplace_back("eql", "", lineCnt);
            output << "eql\n";
            break;
        case '<':
            state = sy_lss;
            break;
        case '>':
            state = sy_gt;
            break;
        case '(':
            state = sy_lpar;
            break;
        case ')':
            tokens.emplace_back("rparent", "", lineCnt);
            output << "rparent\n";
            break;
        case '[':
            tokens.emplace_back("lbrack", "", lineCnt);
            output << "lbrack\n";
            break;
        case ']':
            tokens.emplace_back("rbrack", "", lineCnt);
            output << "rbrack\n";
            break;
        case ',':
            tokens.emplace_back("comma", "", lineCnt);
            output << "comma\n";
            break;
        case ';':
            tokens.emplace_back("semicolon", "", lineCnt);
            output << "semicolon\n\n";
            break;
        case '.':
            state = sy_period; 
            str += c;
            break;
        case ':':
            state = sy_colon;
            break;
    }
    if(state == Start){
        if(isNumber(c)){
            str += c;
            state = Number;
        } else if(isAlphabet(c)){
            str += c;
            state = ident;
        } else if(!isWhitespace(c) && !isSymbol(c)){
            str += c;
            state = Unknown;  
        } 
    }
}

void identBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit, std::vector<Token>& tokens){
    if(isAlphabet(c) || isNumber(c)){
        str += c;
    } else{
        std::string identOrKw = keywordLookup(str);
        if(identOrKw == "ident"){
            tokens.emplace_back("ident", str, lineCnt);
            output << "ident (" << str << ")\n";
        } else{
            tokens.emplace_back(identOrKw, "", lineCnt);
            output << identOrKw << "\n";
        }
        str = "";
        state = Start;
        startBehavior(output, lineCnt, c, state, str, shouldExit, tokens);
    } 
}

std::string keywordLookup(std::string str){
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    if(keywordLookupTable.find(str) == keywordLookupTable.end()){
        return "ident";
    } else{
        return keywordLookupTable[str];
    }
}

std::unordered_map<std::string, std::string> keywordLookupTable = {
    {"array", "arraysy"},
    {"begin", "beginsy"},
    {"case", "casesy"},
    {"const", "constsy"},
    {"div", "idiv"},
    {"do", "dosy"},
    {"downto", "downtosy"},
    {"else", "elsesy"},
    {"end", "endsy"},
    {"for", "forsy"},
    {"function", "functionsy"},
    {"if", "ifsy"},
    {"of", "ofsy"},
    {"procedure", "proceduresy"},
    {"program", "programsy"},
    {"record", "recordsy"},
    {"repeat", "repeatsy"},
    {"then", "thensy"},
    {"to", "tosy"},
    {"type", "typesy"},
    {"until", "untilsy"},
    {"var", "varsy"},
    {"while", "whilesy"},
    {"and", "andsy"},
    {"mod", "imod"},
    {"not", "notsy"},
    {"or", "orsy"},
};