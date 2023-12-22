#include "ast.hpp"
const std::shared_ptr<SimpleType> U32Expr::type =
    std::make_shared<SimpleType>(TypeEnum::I32, RefSpecifier::NON_REF);
const std::shared_ptr<SimpleType> F64Expr::type =
    std::make_shared<SimpleType>(TypeEnum::F64, RefSpecifier::NON_REF);