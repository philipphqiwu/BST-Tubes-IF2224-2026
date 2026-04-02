#include "lexer.hpp"

void lexer(std::ifstream& input, std::ofstream& output){
    int state = Start;
    char c;
    bool shouldExit = false;
    bool stringHadEscape = false;
    (void)stringHadEscape;

    std::string str = "";
    int lineCnt = 1;

    while(input.get(c)){
        if(shouldExit) return;
        if(c == '\n'){
            lineCnt++;
        }
        switch(state){
            // Start/empty
            case Start:
                startBehavior(output, lineCnt, c, state, str, shouldExit);
                break;

            // Komen
            case CommentCurly:
                if(c == '}'){
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
                    output << "comment\n";
                    state = Start;
                } else if(c != '*'){
                    state = CommentStar;
                }
                break;

            // Number
            case Number:
                if(!isNumber(c) && c != '.'){
                    output << "intcon (" << str << ")\n";
                    str = "";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else if(isNumber(c)){
                    str += c;
                } else if(c == '.'){
                    str += c;
                    state = RealBegin;
                }
                break;

            case RealBegin:
                if(!isNumber(c)){
                    errorMsg(output, lineCnt, c, shouldExit);
                } else if(isNumber(c)){
                    str += c;
                    state = Real;
                }
                break;

            case Real:
                if(!isNumber(c)){
                    output << "realcon (" << str << ")\n";
                    str = "";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
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
                    errorMsg(output, lineCnt, c, shouldExit);
                    // return;
                }
                if (c == '\'') {
                    if (input.peek() == '\''){
                        input.get(c);
                        str += '\'';
                        stringHadEscape = true;
                    } else{
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
                identBehavior(output, lineCnt, c, state, str, shouldExit);
                break;

            // Simbol / operator
            case sy_plus:
                output << "plus\n";
                state = Start;
                startBehavior(output, lineCnt, c, state, str, shouldExit);
                break;
            case sy_minus:
                if(isNumber(c)){
                    str = "-";
                    str += c;
                    state = Number;
                } else{
                    output << "minus\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case sy_eql:
                if(c == '='){
                    output << "eql\n";
                    state = Start;
                } else{
                    errorMsg(output, lineCnt, c, shouldExit);
                    return;
                }
                break;
            case sy_lss:
                if(c == '>'){
                    output << "neq\n";
                    state = Start;
                } else if(c == '='){
                    output << "leq\n";
                    state = Start;
                } else if(isWhitespace(c)){
                    output << "lss\n";
                    state = Start;
                } else{
                    output << "lss\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case sy_gt:
                if(c == '='){
                    output << "geq\n";
                    state = Start;
                } else if(isWhitespace(c)){
                    output << "gtr\n";
                    state = Start;
                } else{
                    output << "gtr\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case sy_lpar:
                if(c == '*'){
                    state = CommentStar;
                } else{
                    output << "lparent\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case sy_colon:
                if(c == '='){
                    output << "becomes\n";
                    state = Start;
                } else{
                    output << "colon\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
        }
    }

    // Process at EOF
    if(input.eof() && state != Start){
        if(state == CommentCurly || state == CommentStar || state == CommentStarClose || state == RealBegin || state == CharBegin || state == Char || state == String || state == sy_eql){
            errorMsg(output, lineCnt, c, shouldExit);
        } else if(state == Number){
            output << "intcon (" << str << ")\n";
        } else if(state == Real){
            output << "realcon (" << str << ")\n";
        } else if(state == Char){
            output << "charcon (" << str << ")\n";
        } else if(state == String){
            output << "string ('" << str << "')\n";
        } else if(state == sy_plus){
            output << "plus\n";
        } else if(state == sy_minus){
             output << "minus\n";
        } else if(state == sy_lss){
            output << "lss\n";
        } else if(state == sy_gt){
            output << "gtr\n";
        } else if(state == sy_colon){
            output << "colon\n";
        } else if(state == sy_lpar){
            output << "lparent\n";
        } else {
            std::string identOrKw = keywordLookup(str);
            if(identOrKw == "ident"){
                output << "ident (" << str << ")\n";
            } else{
                output << identOrKw << "\n";
            }
        }
    }

    return;
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
void errorMsg(std::ofstream& output, int lineCnt, char c, bool& shouldExit){
    if (c == '\n'){
        output << "Error at line " << lineCnt-1 << " at newline character \'\\n\' \n";
    } else{
        output << "Error at line " << lineCnt << " at character '" << c << "'\n";
    }
    shouldExit = true;
}
void startBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit){
    switch(c){
        case '{':
            state = CommentCurly;
            break;
        case '\'':
            state = CharBegin;
            break;
        case '+':
            state = sy_plus;
            break;
        case '-':
            state = sy_minus;
            break;
        case '*':
            output << "times\n";
            break;
        case '/':
            output << "rdiv\n";
            break;
        case '=':
            state = sy_eql;
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
            output << "rparent\n";
            break;
        case '[':
            output << "lbrack\n";
            break;
        case ']':
            output << "rbrack\n";
            break;
        case ',':
            output << "comma\n";
            break;
        case ';':
            output << "semicolon\n\n";
            break;
        case '.':
            output << "period\n";
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
            errorMsg(output, lineCnt, c, shouldExit);
            return;
        } 
    }
}

void identBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str, bool& shouldExit){
    if(isAlphabet(c) || isNumber(c)){
        str += c;
    } else{
        std::string identOrKw = keywordLookup(str);
        if(identOrKw == "ident"){
            output << "ident (" << str << ")\n";
        } else{
            output << identOrKw << "\n";
        }
        str = "";
        state = Start;
        startBehavior(output, lineCnt, c, state, str, shouldExit);
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