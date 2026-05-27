#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "interpreter/header/icgen.hpp"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class Interpreter
{
public:
    Interpreter(const std::vector<ICInstr> &code,
                const SymbolTable &symtab,
                const std::unordered_map<int, int> &subprogStartLine);

    bool execute(std::ostream &out);
    const std::vector<std::string> &errors() const { return errors_; }

    struct Value
    {
        enum class Kind
        {
            Number,
            String,
            Char
        };
        Kind kind = Kind::Number;
        long long number = 0;
        std::string text;

        static Value fromNumber(long long v);
        static Value fromString(const std::string &s);
        static Value fromChar(long long v);
    };

    struct CallInfo
    {
        int param_count = 0;
        bool is_function = false;
        int lex_level = 0;
    };

    struct FrameMeta
    {
        int bp = 0;
        bool is_function = false;
        bool has_return = false;
        Value return_value = Value::fromNumber(0);
        int lex_level = 0;
    };

private:
    const std::vector<ICInstr> *code_;
    std::unordered_map<int, CallInfo> call_info_;
    std::vector<std::string> errors_;
    std::unordered_map<int, std::unordered_map<int, int>> addr_type_;

    std::vector<Value> stack_;
    int bp_ = 0;
    int sp_ = -1;
    int pc_ = 0;

    std::vector<FrameMeta> frames_;

    int base(int level) const;
    bool ensureIndex(int index);

    bool push(const Value &v);
    bool pop(Value &v);
    bool popNumber(long long &v);

    std::string valueToString(const Value &v) const;
    int lookupType(int level, int address) const;

    bool execInstruction(const ICInstr &ins, std::ostream &out);
    void addError(const std::string &msg);
};

#endif
