#include <iostream>
#include <string>
#include <any>

using std::string;

/**
 * Visitor 패턴 예시 : 트리를 순회하여 문자열을 반환할 것인지 평가식의 결과를 반환할 것인지를 결정하기
 * 
 * 인터프리터 구현의 일부
 *
 * !! 구현의 편의성을 위해 메모리 해제는 하지 않음 !!
 * !! 실제 사용할 때는 shared_ptr이나 레퍼런스 카운팅을 직접 구현해서 쓰도록 한다 !!
 */
void PrintAny(std::any value)
{
    if (value.type() == typeid(std::nullptr_t))
    {
        std::cout << "nullptr_t\n";
    }
    else if (value.type() == typeid(string))
    {
        std::cout << "string : " << std::any_cast<string>(value) << '\n';
    }
    else if (value.type() == typeid(int))
    {
        std::cout << "int : " << std::any_cast<int>(value) << '\n';
    }
}

struct Expr
{
    virtual ~Expr() = default;

    virtual std::any Accept(class ExprVisitor* visitor) = 0;
};

class ExprVisitor
{
public:
    virtual ~ExprVisitor() = default;

public:
    std::any Run(Expr* expr)
    {
        return expr->Accept(this);
    }

public:
    virtual std::any VisitBinaryExpr(struct Binary* binaryExpr) = 0;
    virtual std::any VisitLiteralExpr(struct Literal* literalExpr) = 0;
};

struct Binary : Expr
{
    char  oper;
    Expr* left;
    Expr* right;

    Binary(char oper, Expr* left, Expr* right)
        : oper(oper), left(left), right(right)
    { }

    std::any Accept(ExprVisitor* visitor) override
    {
        return visitor->VisitBinaryExpr(this);
    }
};

struct Literal : Expr
{
    int value;

    Literal(int value)
        : value(value)
    { }

    std::any Accept(ExprVisitor* visitor) override
    {
        return visitor->VisitLiteralExpr(this);
    }
};

class Printer : public ExprVisitor
{
public:
    std::any VisitBinaryExpr(Binary* binaryExpr) override
    {
        string ret = "(";

        ret += binaryExpr->oper;

        ret += " ";
        ret += std::any_cast<string>(binaryExpr->left->Accept(this));

        ret += " ";
        ret += std::any_cast<string>(binaryExpr->right->Accept(this));

        ret += ")";

        return ret;
    }

    std::any VisitLiteralExpr(Literal* literalExpr) override
    {
        return std::to_string(literalExpr->value);
    }
};

class Interpreter : public ExprVisitor
{
public:
    std::any VisitBinaryExpr(Binary* binaryExpr) override
    {
        int left  = std::any_cast<int>(this->Run(binaryExpr->left));
        int right = std::any_cast<int>(this->Run(binaryExpr->right));

        switch (binaryExpr->oper)
        {
            case '+': return left + right;
            case '-': return left - right;
            case '*': return left * right;
            case '/': return left / right;
        }

        return nullptr;
    }

    std::any VisitLiteralExpr(Literal* literalExpr) override
    {
        return literalExpr->value;
    }
};

int main()
{
    Expr* expr = nullptr;

    Printer     printer;
    Interpreter interpreter;

    PrintAny(nullptr);
    
    // (10 + 20) * (5 - 3)
    expr = new Binary{
        '*', new Binary{ '+', new Literal{ 10 }, new Literal { 20 } }, new Binary{ '-', new Literal{ 5 }, new Literal{ 3 } } };

    PrintAny(printer.Run(expr));
    PrintAny(interpreter.Run(expr));

    // 100 / (16 + 3)
    expr = new Binary{
        '/', new Literal{ 100 }, new Binary{ '+', new Literal{ 16 }, new Literal { 3 } } };

    PrintAny(printer.Run(expr));
    PrintAny(interpreter.Run(expr));

    // 100 / (16 + 5)
    expr = new Binary{
        '/', new Literal{ 100 }, new Binary{ '+', new Literal{ 16 }, new Literal { 5 } } };

    PrintAny(printer.Run(expr));
    PrintAny(interpreter.Run(expr));

    return 0;
}
