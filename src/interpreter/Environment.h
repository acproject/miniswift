#ifndef MINISWIFT_ENVIRONMENT_H
#define MINISWIFT_ENVIRONMENT_H

#include "Value.h"
#include "../lexer/Token.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace miniswift {

struct Variable {
    Value value;
    bool isConst;
    std::string typeName;
};

class Environment {
public:
    Environment();
    explicit Environment(std::shared_ptr<Environment> enclosing);

    void define(const std::string& name, const Value& value, bool isConst, const std::string& typeName);
    Value get(const Token& name);
    void assign(const Token& name, const Value& value);

private:
    std::shared_ptr<Environment> enclosing;
    std::unordered_map<std::string, Variable> values;
};

} // namespace miniswift



#endif // MINISWIFT_ENVIRONMENT_H