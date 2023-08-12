#include "ast.hpp"
const std::shared_ptr<SimpleType> I32Expr::type =
    std::make_shared<SimpleType>(TypeEnum::I32);
const std::shared_ptr<SimpleType> F64Expr::type =
    std::make_shared<SimpleType>(TypeEnum::F64);