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

            case kw_a:
                if(c == 'r'){
                    str += c;
                    state = kw_ar;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_ar:
                if(c == 'r'){
                    str += c;
                    state = kw_arr;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_arr:
                if(c == 'a'){
                    str += c;
                    state = kw_arra;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_arra:
                if(c == 'y'){
                    str += c;
                    state = kw_array;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_array:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "arraysy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_b:
                if(c == 'e'){
                    str += c;
                    state = kw_be;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_be:
                if(c == 'g'){
                    str += c;
                    state = kw_beg;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_beg:
                if(c == 'i'){
                    str += c;
                    state = kw_begi;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_begi:
                if(c == 'n'){
                    str += c;
                    state = kw_begin;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_begin:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "beginsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_c:
                if(c == 'a'){
                    str += c;
                    state = kw_ca;
                } else if(c == 'o'){
                    str += c;
                    state = kw_co;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_ca:
                if(c == 's'){
                    str += c;
                    state = kw_cas;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_cas:
                if(c == 'e'){
                    str += c;
                    state = kw_case;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_case:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "casesy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_co:
                if(c == 'n'){
                    str += c;
                    state = kw_con;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_con:
                if(c == 's'){
                    str += c;
                    state = kw_cons;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_cons:
                if(c == 't'){
                    str += c;
                    state = kw_const;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_const:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "constsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_d:
                if(c == 'i'){
                    str += c;
                    state = kw_di;
                } else if(c == 'o'){
                    str += c;
                    state = kw_do;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_di:
                if(c == 'v'){
                    str += c;
                    state = kw_div;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_do:
                if(c == 'w'){
                    str += c;
                    state = kw_dow;
                } else if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "dosy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                }else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_dow:
                if(c == 'n'){
                    str += c;
                    state = kw_down;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_down:
                if(c == 't'){
                    str += c;
                    state = kw_downt;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_downt:
                if(c == 'o'){
                    str += c;
                    state = kw_downto;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_downto:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "downtosy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_div:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "idiv\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_e:
                if(c == 'l'){
                    str += c;
                    state = kw_el;
                } else if(c == 'n'){
                    str += c;
                    state = kw_en;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_en:
                if(c == 'd'){
                    str += c;
                    state = kw_end;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_end:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "endsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_el:
                if(c == 's'){
                    str += c;
                    state = kw_els;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_els:
                if(c == 'e'){
                    str += c;
                    state = kw_else;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_else:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "elsesy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_f:
                if(c == 'o'){
                    str += c;
                    state = kw_fo;
                } else if(c == 'u'){
                    str += c;
                    state = kw_fu;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_fo:
                if(c == 'r'){
                    str += c;
                    state = kw_for;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_for:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "forsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_fu:
                if(c == 'n'){
                    str += c;
                    state = kw_fun;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_fun:
                if(c == 'c'){
                    str += c;
                    state = kw_func;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_func:
                if(c == 't'){
                    str += c;
                    state = kw_funct;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_funct:
                if(c == 'i'){
                    str += c;
                    state = kw_functi;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_functi:
                if(c == 'o'){
                    str += c;
                    state = kw_functio;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_functio:
                if(c == 'n'){
                    str += c;
                    state = kw_function;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_function:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "functionsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_i:
                if(c == 'f'){
                    str += c;
                    state = kw_if;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_if:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "ifsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_o:
                if(c == 'f'){
                    str += c;
                    state = kw_of;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_of:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "ofsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_p:
                if(c == 'r'){
                    str += c;
                    state = kw_pr;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_pr:
                if(c == 'o'){
                    str += c;
                    state = kw_pro;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_pro:
                if(c == 'c'){
                    str += c;
                    state = kw_proc;
                } else if(c == 'g'){
                    str += c;
                    state = kw_prog;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_prog:
                if(c == 'r'){
                    str += c;
                    state = kw_progr;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_progr:
                if(c == 'a'){
                    str += c;
                    state = kw_progra;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_progra:
                if(c == 'm'){
                    str += c;
                    state = kw_program;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_program:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "programsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_proc:
                if(c == 'e'){
                    str += c;
                    state = kw_proce;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_proce:
                if(c == 'd'){
                    str += c;
                    state = kw_proced;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_proced:
                if(c == 'u'){
                    str += c;
                    state = kw_procedu;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_procedu:
                if(c == 'r'){
                    str += c;
                    state = kw_procedur;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_procedur:
                if(c == 'e'){
                    str += c;
                    state = kw_procedure;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_procedure:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "proceduresy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_r:
                if(c == 'e'){
                    str += c;
                    state = kw_re;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_re:
                if(c == 'c'){
                    str += c;
                    state = kw_rec;
                } else if(c == 'p'){
                    str += c;
                    state = kw_rep;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_rec:
                if(c == 'o'){
                    str += c;
                    state = kw_reco;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_reco:
                if(c == 'r'){
                    str += c;
                    state = kw_recor;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_recor:
                if(c == 'd'){
                    str += c;
                    state = kw_record;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_record:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "recordsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_rep:
                if(c == 'e'){
                    str += c;
                    state = kw_repe;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_repe:
                if(c == 'a'){
                    str += c;
                    state = kw_repea;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_repea:
                if(c == 't'){
                    str += c;
                    state = kw_repeat;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_repeat:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "repeatsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_t:
                if(c == 'h'){
                    str += c;
                    state = kw_th;
                } else if(c == 'o'){
                    str += c;
                    state = kw_to;
                } else if(c == 'y'){
                    str += c;
                    state = kw_ty;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_th:
                if(c == 'e'){
                    str += c;
                    state = kw_the;
                } else if(c == 'p'){
                    str += c;
                    state = kw_rep;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_the:
                if(c == 'n'){
                    str += c;
                    state = kw_then;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_then:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "thensy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_to:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "tosy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_ty:
                if(c == 'p'){
                    str += c;
                    state = kw_typ;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_typ:
                if(c == 'e'){
                    str += c;
                    state = kw_type;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_type:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "typesy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;


            case kw_u:
                if(c == 'n'){
                    str += c;
                    state = kw_un;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_un:
                if(c == 't'){
                    str += c;
                    state = kw_unt;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_unt:
                if(c == 'i'){
                    str += c;
                    state = kw_unti;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_unti:
                if(c == 'l'){
                    str += c;
                    state = kw_until;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_until:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "untilsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_v:
                if(c == 'a'){
                    str += c;
                    state = kw_va;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_va:
                if(c == 'r'){
                    str += c;
                    state = kw_var;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_var:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "varsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_w:
                if(c == 'h'){
                    str += c;
                    state = kw_wh;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_wh:
                if(c == 'i'){
                    str += c;
                    state = kw_whi;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_whi:
                if(c == 'l'){
                    str += c;
                    state = kw_whil;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_whil:
                if(c == 'e'){
                    str += c;
                    state = kw_while;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_while:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "whilesy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            case kw_A:
                if(c == 'N'){
                    str += c;
                    state = kw_AN;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_AN:
                if(c == 'D'){
                    str += c;
                    state = kw_AND;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_AND:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "andsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            
            case kw_M:
                if(c == 'O'){
                    str += c;
                    state = kw_MO;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_MO:
                if(c == 'D'){
                    str += c;
                    state = kw_MOD;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_MOD:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "imod\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

             case kw_N:
                if(c == 'O'){
                    str += c;
                    state = kw_NO;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_NO:
                if(c == 'T'){
                    str += c;
                    state = kw_NOT;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_NOT:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "notsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

             case kw_O:
                if(c == 'R'){
                    str += c;
                    state = kw_OR;
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;
            case kw_OR:
                if(!isAlphabet(c) && !isNumber(c)){
                    str = "";
                    output << "orsy\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                } else{
                    state = ident;
                    identBehavior(output, lineCnt, c, state, str, shouldExit);
                }
                break;

            // Simbol / operator
            case sy_plus:
                if(isNumber(c)){
                    str = c;
                    state = Number;
                } else{
                    output << "plus\n";
                    state = Start;
                    startBehavior(output, lineCnt, c, state, str, shouldExit);
                }
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
        if(state == CommentCurly || state == CommentStar || state == CommentStarClose || state == RealBegin || state == CharBegin || state == sy_eql){
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
        } else if(state == kw_array){
            output << "arraysy\n";
        } else if(state == kw_begin){
            output << "beginsy\n";
        } else if(state == kw_case){
            output << "casesy\n";
        } else if(state == kw_const){
            output << "constsy\n";
        }else if(state == kw_div){
            output << "divsy\n";
        } else if(state == kw_do){
            output << "dosy\n";
        } else if(state == kw_downto){
            output << "downtosy\n";
        } else if(state == kw_else){
            output << "elsesy\n";
        } else if(state == kw_end){
            output << "endsy\n";
        } else if(state == kw_for){
            output << "forsy\n";
        } else if(state == kw_function){
            output << "functionsy\n";
        } else if(state == kw_if){
            output << "ifsy\n";
        } else if(state == kw_of){
            output << "ofsy\n";
        } else if(state == kw_procedure){
            output << "proceduresy\n";
        } else if(state == kw_program){
            output << "programsy\n";
        } else if(state == kw_record){
            output << "recordsy\n";
        } else if(state == kw_repeat){
            output << "repeatsy\n";
        } else if(state == kw_then){
            output << "thensy\n";
        } else if(state == kw_to){
            output << "tosy\n";
        } else if(state == kw_type){
            output << "typesy\n";
        } else if(state == kw_until){
            output << "untilsy\n";
        } else if(state == kw_var){
            output << "varsy\n";
        } else if(state == kw_while){
            output << "whilesy\n";
        } else if(state == kw_AND){
            output << "andsy\n";
        } else if(state == kw_MOD){
            output << "imod\n";
        } else if(state == kw_NOT){
            output << "notsy\n";
        } else if(state == kw_OR){
            output << "orsy\n";
        } else {
            output << "ident (" << str << ")\n";
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
    output << "Error at line " << lineCnt << " at character '" << c << "'\n";
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
        output << "ident (" << str << ")\n";
        str = "";
        state = Start;
        startBehavior(output, lineCnt, c, state, str, shouldExit);
    } 
}