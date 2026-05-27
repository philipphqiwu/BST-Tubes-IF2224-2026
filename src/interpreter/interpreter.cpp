#include "interpreter/header/interpreter.hpp"

#include <algorithm>

namespace
{

    std::string normalizeStringLiteral(const std::string &raw)
    {
        if (raw.size() >= 2 && raw.front() == '\'' && raw.back() == '\'')
        {
            std::string inner = raw.substr(1, raw.size() - 2);
            std::string out;
            out.reserve(inner.size());
            for (size_t i = 0; i < inner.size(); ++i)
            {
                if (inner[i] == '\'' && i + 1 < inner.size() && inner[i + 1] == '\'')
                {
                    out.push_back('\'');
                    ++i;
                }
                else
                {
                    out.push_back(inner[i]);
                }
            }
            return out;
        }
        return raw;
    }

    bool isTruthy(const Interpreter::Value &v)
    {
        if (v.kind == Interpreter::Value::Kind::String)
        {
            return !v.text.empty();
        }
        return v.number != 0;
    }

}

Interpreter::Value Interpreter::Value::fromNumber(long long v)
{
    Value out;
    out.kind = Kind::Number;
    out.number = v;
    return out;
}

Interpreter::Value Interpreter::Value::fromString(const std::string &s)
{
    Value out;
    out.kind = Kind::String;
    out.text = s;
    return out;
}

Interpreter::Value Interpreter::Value::fromChar(long long v)
{
    Value out;
    out.kind = Kind::Char;
    out.number = v;
    return out;
}

Interpreter::Interpreter(const std::vector<ICInstr> &code,
                         const SymbolTable &symtab,
                         const std::unordered_map<int, int> &subprogStartLine)
    : code_(&code)
{
    for (const auto &pair : subprogStartLine)
    {
        int tab_idx = pair.first;
        int start_line = pair.second;
        if (tab_idx < 0 || tab_idx >= (int)symtab.tab.size())
            continue;
        const TabEntry &entry = symtab.tab[tab_idx];
        CallInfo info;
        info.is_function = (entry.obj == ObjClass::FUNCTION);
        info.lex_level = entry.lev + 1;
        int btab_idx = entry.ref;
        if (btab_idx >= 0 && btab_idx < (int)symtab.btab.size())
        {
            info.param_count = symtab.btab[btab_idx].psze;
        }
        call_info_[start_line] = info;
    }

    for (const auto &entry : symtab.tab)
    {
        if (entry.obj != ObjClass::VARIABLE)
            continue;
        addr_type_[entry.lev][entry.adr] = entry.type;
    }
}

void Interpreter::addError(const std::string &msg)
{
    errors_.push_back(msg);
}

int Interpreter::base(int level) const
{
    int b = bp_;
    int steps = level;
    while (steps > 0)
    {
        if (b < 0 || b >= (int)stack_.size())
            return -1;
        const Value &v = stack_[b];
        if (v.kind != Value::Kind::Number)
            return -1;
        b = static_cast<int>(v.number);
        --steps;
    }
    return b;
}

bool Interpreter::ensureIndex(int index)
{
    if (index < 0)
        return false;
    if (index >= (int)stack_.size())
    {
        stack_.resize(index + 1, Value::fromNumber(0));
    }
    return true;
}

bool Interpreter::push(const Value &v)
{
    if (!ensureIndex(sp_ + 1))
    {
        addError("Stack overflow");
        return false;
    }
    ++sp_;
    stack_[sp_] = v;
    return true;
}

bool Interpreter::pop(Value &v)
{
    if (sp_ < 0)
    {
        addError("Stack underflow");
        return false;
    }
    v = stack_[sp_];
    --sp_;
    return true;
}

bool Interpreter::popNumber(long long &v)
{
    Value tmp;
    if (!pop(tmp))
        return false;
    if (tmp.kind != Value::Kind::Number && tmp.kind != Value::Kind::Char)
    {
        addError("Expected numeric value on stack");
        return false;
    }
    v = tmp.number;
    return true;
}

std::string Interpreter::valueToString(const Value &v) const
{
    if (v.kind == Value::Kind::String)
        return v.text;
    if (v.kind == Value::Kind::Char)
        return std::string(1, static_cast<char>(v.number));
    return std::to_string(v.number);
}

int Interpreter::lookupType(int level, int address) const
{
    auto it = addr_type_.find(level);
    if (it == addr_type_.end())
        return 0;
    auto jt = it->second.find(address);
    if (jt == it->second.end())
        return 0;
    return jt->second;
}

bool Interpreter::execute(std::ostream &out)
{
    errors_.clear();
    stack_.clear();
    frames_.clear();

    bp_ = 0;
    sp_ = -1;
    pc_ = 0;

    frames_.push_back(FrameMeta{});

    while (pc_ >= 0 && pc_ < (int)code_->size())
    {
        const ICInstr &ins = (*code_)[pc_];
        if (!execInstruction(ins, out))
            return false;
    }

    return errors_.empty();
}

bool Interpreter::execInstruction(const ICInstr &ins, std::ostream &out)
{
    if (ins.op == "LIT")
    {
        if (ins.has_str)
        {
            if (!push(Value::fromString(normalizeStringLiteral(ins.str_operand))))
                return false;
        }
        else
        {
            if (!push(Value::fromNumber(ins.operand)))
                return false;
        }
        ++pc_;
        return true;
    }

    if (ins.op == "INT")
    {
        int needed = bp_ + static_cast<int>(ins.operand);
        if (!ensureIndex(needed - 1))
        {
            addError("Invalid INT size");
            return false;
        }
        if (sp_ < needed - 1)
            sp_ = needed - 1;
        ++pc_;
        return true;
    }

    if (ins.op == "LOD")
    {
        if (ins.level == 0 && ins.operand == 0 && !frames_.empty())
        {
            if (frames_.back().has_return)
            {
                if (!push(frames_.back().return_value))
                    return false;
                ++pc_;
                return true;
            }
            if (frames_.back().is_function)
            {
                if (!push(Value::fromNumber(0)))
                    return false;
                ++pc_;
                return true;
            }
        }

        int b = base(ins.level);
        if (b < 0)
        {
            addError("Invalid base for LOD");
            return false;
        }
        int addr = b + static_cast<int>(ins.operand);
        if (addr < 0 || addr >= (int)stack_.size())
        {
            addError("LOD address out of bounds");
            return false;
        }
        Value loaded = stack_[addr];
        int current_level = frames_.empty() ? 0 : frames_.back().lex_level;
        int target_level = current_level - ins.level;
        if (target_level < 0)
            target_level = 0;
        int type_id = lookupType(target_level, static_cast<int>(ins.operand));
        if (type_id == 4 && loaded.kind == Value::Kind::Number)
        {
            loaded = Value::fromChar(loaded.number);
        }
        if (!push(loaded))
            return false;
        ++pc_;
        return true;
    }

    if (ins.op == "STO")
    {
        Value v;
        if (!pop(v))
            return false;

        if (ins.level == 0 && ins.operand == 0 && !frames_.empty())
        {
            frames_.back().return_value = v;
            frames_.back().has_return = true;
            ++pc_;
            return true;
        }

        int b = base(ins.level);
        if (b < 0)
        {
            addError("Invalid base for STO");
            return false;
        }
        int addr = b + static_cast<int>(ins.operand);
        if (!ensureIndex(addr))
        {
            addError("STO address out of bounds");
            return false;
        }
        int current_level = frames_.empty() ? 0 : frames_.back().lex_level;
        int target_level = current_level - ins.level;
        if (target_level < 0)
            target_level = 0;
        int type_id = lookupType(target_level, static_cast<int>(ins.operand));
        if (type_id == 4 && v.kind == Value::Kind::Number)
        {
            v = Value::fromChar(v.number);
        }
        stack_[addr] = v;
        ++pc_;
        return true;
    }

    if (ins.op == "JMP")
    {
        int target = static_cast<int>(ins.operand);
        if (target < 0 || target >= (int)code_->size())
        {
            addError("Invalid JMP target");
            return false;
        }
        pc_ = target;
        return true;
    }

    if (ins.op == "JPC")
    {
        Value cond;
        if (!pop(cond))
            return false;
        if (!isTruthy(cond))
        {
            int target = static_cast<int>(ins.operand);
            if (target < 0 || target >= (int)code_->size())
            {
                addError("Invalid JPC target");
                return false;
            }
            pc_ = target;
        }
        else
        {
            ++pc_;
        }
        return true;
    }

    if (ins.op == "CAL")
    {
        int target = static_cast<int>(ins.operand);
        if (target < 0 || target >= (int)code_->size())
        {
            addError("Invalid CAL target");
            return false;
        }

        CallInfo info;
        auto it = call_info_.find(target);
        if (it != call_info_.end())
            info = it->second;

        int arg_count = info.param_count;
        if (arg_count < 0)
            arg_count = 0;
        if (arg_count > sp_ + 1)
        {
            addError("Not enough arguments for CAL");
            return false;
        }

        int base_index = (arg_count == 0) ? (sp_ + 1) : (sp_ - arg_count + 1);
        int static_link = base(ins.level);
        if (static_link < 0)
        {
            addError("Invalid static link for CAL");
            return false;
        }

        if (!ensureIndex(sp_ + 3))
        {
            addError("Stack overflow during CAL");
            return false;
        }

        for (int i = sp_; i >= base_index; --i)
        {
            stack_[i + 3] = stack_[i];
        }
        sp_ += 3;

        stack_[base_index] = Value::fromNumber(static_link);
        stack_[base_index + 1] = Value::fromNumber(bp_);
        stack_[base_index + 2] = Value::fromNumber(pc_ + 1);

        bp_ = base_index;
        frames_.push_back(FrameMeta{bp_, info.is_function, false, Value::fromNumber(0), info.lex_level});
        pc_ = target;
        return true;
    }

    if (ins.op == "RET")
    {
        if (frames_.size() <= 1)
        {
            pc_ = static_cast<int>(code_->size());
            return true;
        }

        FrameMeta frame = frames_.back();
        frames_.pop_back();

        if (bp_ + 2 >= (int)stack_.size())
        {
            addError("Return address out of bounds");
            return false;
        }
        const Value &ra_val = stack_[bp_ + 2];
        const Value &dl_val = stack_[bp_ + 1];
        if (ra_val.kind != Value::Kind::Number || dl_val.kind != Value::Kind::Number)
        {
            addError("Invalid return address or dynamic link");
            return false;
        }

        int return_addr = static_cast<int>(ra_val.number);
        int caller_bp = static_cast<int>(dl_val.number);

        int old_bp = bp_;
        bp_ = caller_bp;
        sp_ = old_bp - 1;

        if (frame.has_return || frame.is_function)
        {
            Value ret_val = frame.has_return ? frame.return_value : Value::fromNumber(0);
            if (!push(ret_val))
                return false;
        }

        if (return_addr < 0 || return_addr > (int)code_->size())
        {
            addError("Invalid return address");
            return false;
        }
        pc_ = return_addr;
        return true;
    }

    if (ins.op == "OPR")
    {
        int op = static_cast<int>(ins.operand);
        if (op == 1)
        {
            long long v = 0;
            if (!popNumber(v))
                return false;
            if (!push(Value::fromNumber(-v)))
                return false;
        }
        else if (op == 2 || op == 3 || op == 4 || op == 5 || op == 6)
        {
            Value right;
            Value left;
            if (!pop(right) || !pop(left))
                return false;

            if (left.kind == Value::Kind::String || right.kind == Value::Kind::String)
            {
                if (op == 2 && left.kind == Value::Kind::String && right.kind == Value::Kind::String)
                {
                    if (!push(Value::fromString(left.text + right.text)))
                        return false;
                }
                else
                {
                    addError("Invalid string operation");
                    return false;
                }
            }
            else
            {
                long long a = left.number;
                long long b = right.number;
                long long result = 0;
                if (op == 2)
                    result = a + b;
                if (op == 3)
                    result = a - b;
                if (op == 4)
                    result = a * b;
                if (op == 5)
                {
                    if (b == 0)
                    {
                        addError("Division by zero");
                        return false;
                    }
                    result = a / b;
                }
                if (op == 6)
                {
                    if (b == 0)
                    {
                        addError("Modulo by zero");
                        return false;
                    }
                    result = a % b;
                }
                if (!push(Value::fromNumber(result)))
                    return false;
            }
        }
        else if (op >= 7 && op <= 12)
        {
            Value right;
            Value left;
            if (!pop(right) || !pop(left))
                return false;

            bool cmp = false;
            if (left.kind == Value::Kind::String || right.kind == Value::Kind::String)
            {
                if (left.kind != Value::Kind::String || right.kind != Value::Kind::String)
                {
                    addError("Invalid comparison between string and number");
                    return false;
                }
                if (op == 7)
                    cmp = (left.text == right.text);
                if (op == 8)
                    cmp = (left.text != right.text);
                if (op == 9)
                    cmp = (left.text < right.text);
                if (op == 10)
                    cmp = (left.text >= right.text);
                if (op == 11)
                    cmp = (left.text > right.text);
                if (op == 12)
                    cmp = (left.text <= right.text);
            }
            else
            {
                long long a = left.number;
                long long b = right.number;
                if (op == 7)
                    cmp = (a == b);
                if (op == 8)
                    cmp = (a != b);
                if (op == 9)
                    cmp = (a < b);
                if (op == 10)
                    cmp = (a >= b);
                if (op == 11)
                    cmp = (a > b);
                if (op == 12)
                    cmp = (a <= b);
            }
            if (!push(Value::fromNumber(cmp ? 1 : 0)))
                return false;
        }
        else if (op == 13 || op == 14)
        {
            if (sp_ >= 0)
            {
                Value v;
                if (!pop(v))
                    return false;
                out << valueToString(v);
            }
            else if (op == 13)
            {
                addError("WRT needs a value on stack");
                return false;
            }
            if (op == 14)
                out << "\n";
        }
        else if (op == 15 || op == 16)
        {
            Value right;
            Value left;
            if (!pop(right) || !pop(left))
                return false;
            bool res = (op == 15) ? (isTruthy(left) && isTruthy(right))
                                  : (isTruthy(left) || isTruthy(right));
            if (!push(Value::fromNumber(res ? 1 : 0)))
                return false;
        }
        else if (op == 17)
        {
            Value v;
            if (!pop(v))
                return false;
            bool res = !isTruthy(v);
            if (!push(Value::fromNumber(res ? 1 : 0)))
                return false;
        }
        else if (op == 18)
        {
            long long addr = 0;
            if (!popNumber(addr))
                return false;
            int index = bp_ + static_cast<int>(addr);
            if (index < 0 || index >= (int)stack_.size())
            {
                addError("Indirect load address out of bounds");
                return false;
            }
            if (!push(stack_[index]))
                return false;
        }
        else if (op == 19)
        {
            long long addr = 0;
            if (!popNumber(addr))
                return false;
            Value v;
            if (!pop(v))
                return false;
            int index = bp_ + static_cast<int>(addr);
            if (!ensureIndex(index))
            {
                addError("Indirect store address out of bounds");
                return false;
            }
            stack_[index] = v;
        }
        else
        {
            addError("Unknown OPR code");
            return false;
        }

        ++pc_;
        return true;
    }

    addError("Unknown instruction: " + ins.op);
    return false;
}
