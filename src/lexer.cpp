#include "lexer.hpp"

void lexer(std::ifstream& input, std::ofstream& output){
    int state = Start;
    char c;

    std::string str = "";
    int lineCnt = 1;

    while(input.get(c)){
        if(c == '\n'){
            lineCnt++;
        }
        switch(state){
            // Start/empty
            case Start:
                startBehavior(output, lineCnt, c, state, str);
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
                    startBehavior(output, lineCnt, c, state, str);
                } else if(isNumber(c)){
                    str += c;
                } else if(c == '.'){
                    str += c;
                    state = Real;
                }
                break;

            case Real:
                if(!isNumber(c)){
                    output << "realcon (" << str << ")\n";
                    str = "";
                    startBehavior(output, lineCnt, c, state, str);
                } else if(isNumber(c)){
                    str += c;
                }
                break;

            // Char / string
            case CharBegin:
                if(c != '\''){
                    str += c;
                    state = Char;
                } else{
                    errorMsg(output, lineCnt, c);
                    return;
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
                if(c == '\''){
                    output << "string ('" << str << "')\n";
                    str = "";
                    state = Start;
                } else {
                    str += c;
                }
                break;

            // Keyword / ident
            case ident:
                identBehavior(output, lineCnt, c, state, str);
                break;

            case kw_a:
                if(c == 'r'){
                    str += c;
                    state = kw_ar;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_ar:
                if(c == 'r'){
                    str += c;
                    state = kw_arr;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_arr:
                if(c == 'a'){
                    str += c;
                    state = kw_arr;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_arra:
                if(c == 'y'){
                    str += c;
                    state = kw_array;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_array:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "arraysy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;

            case kw_b:
                if(c == 'e'){
                    str += c;
                    state = kw_be;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_be:
                if(c == 'g'){
                    str += c;
                    state = kw_beg;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_beg:
                if(c == 'i'){
                    str += c;
                    state = kw_begi;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_begi:
                if(c == 'n'){
                    str += c;
                    state = kw_begin;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_begin:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "beginsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;



            case kw_e:
                if(c == 'n'){
                    str += c;
                    state = kw_en;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_en:
                if(c == 'd'){
                    str += c;
                    state = kw_end;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_end:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "endsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;


            case kw_i:
                if(c == 'f'){
                    str += c;
                    state = kw_if;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_if:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "ifsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;


            case kw_p:
                if(c == 'r'){
                    str += c;
                    state = kw_pr;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_pr:
                if(c == 'o'){
                    str += c;
                    state = kw_pro;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_pro:
                if(c == 'g'){
                    str += c;
                    state = kw_prog;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_prog:
                if(c == 'r'){
                    str += c;
                    state = kw_progr;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_progr:
                if(c == 'a'){
                    str += c;
                    state = kw_progra;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_progra:
                if(c == 'm'){
                    str += c;
                    state = kw_program;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_program:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "programsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;


            case kw_v:
                if(c == 'a'){
                    str += c;
                    state = kw_va;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_va:
                if(c == 'r'){
                    str += c;
                    state = kw_var;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_var:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "varsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;

            case kw_w:
                if(c == 'h'){
                    str += c;
                    state = kw_wh;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_wh:
                if(c == 'i'){
                    str += c;
                    state = kw_whi;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_whi:
                if(c == 'l'){
                    str += c;
                    state = kw_whil;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_whil:
                if(c == 'h'){
                    str += c;
                    state = kw_while;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;
            case kw_while:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "whilesy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str);
                }
                break;

            // Simbol / operator
            case sy_plus:
                if(isNumber(c)){
                    state = Number;
                } else{
                    output << "plus\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                }
                break;
            case sy_minus:
                if(isNumber(c)){
                    str += c;
                    state = Number;
                } else{
                    output << "minus\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                }
                break;
            case sy_eql:
                if(c == '='){
                    output << "eql\n";
                    state = Start;
                } else{
                    errorMsg(output, lineCnt, c);
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
                    startBehavior(output, lineCnt, c, state, str);
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
                    startBehavior(output, lineCnt, c, state, str);
                }
                break;
            case sy_lpar:
                if(c == '*'){
                    state = CommentStar;
                } else{
                    output << "lparent\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                }
                break;
            case sy_colon:
                if(c == '='){
                    output << "becomes\n";
                    state = Start;
                } else{
                    output << "colon\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str);
                }
                break;
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
void errorMsg(std::ofstream& output, int lineCnt, char c){
    output << "Error at line " << lineCnt << " at character '" << c << "'\n";
}
void startBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str){
    switch(c){
        case '{':
            state = CommentCurly;
            break;
        case '\'':
            state = CharBegin;
            break;
        case 'a':
            str += c;
            state = kw_a;
            break;
        case 'b':
            str += c;
            state = kw_b;
            break;
        case 'c':
            str += c;
            state = kw_c;
            break;
        case 'd':
            str += c;
            state = kw_d;
            break;
        case 'e':
            str += c;
            state = kw_e;
            break;
        case 'f':
            str += c;
            state = kw_f;
            break;
        case 'i':
            str += c;
            state = kw_i;
            break;
        case 'o':
            str += c;
            state = kw_o;
            break;
        case 'p':
            str += c;
            state = kw_p;
            break;
        case 'r':
            str += c;
            state = kw_r;
            break;
        case 't':
            str += c;
            state = kw_t;
            break;
        case 'u':
            str += c;
            state = kw_u;
            break;
        case 'v':
            str += c;
            state = kw_v;
            break;
        case 'A':
            str += c;
            state = kw_A;
            break;
        case 'M':
            str += c;
            state = kw_M;
            break;
        case 'N':
            str += c;
            state = kw_N;
            break;
        case 'O':
            str += c;
            state = kw_O;
            break;
        case '+':
            state = sy_plus;
            break;
        case '-':
            str += c;
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
            errorMsg(output, lineCnt, c);
            return;
        } 
    }
}

void identBehavior(std::ofstream& output, int lineCnt, char c, int& state, std::string& str){
    if(isAlphabet(c) || isNumber(c)){
        str += c;
    } else{
        output << "ident (" << str << ")\n";
        str = "";
        state = Start;
        startBehavior(output, lineCnt, c, state, str);
    } 
}