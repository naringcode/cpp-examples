#include <iostream>
#include <string>

#include <vector>
#include <map>

#include <functional>

using std::string;

/**
 * https://en.m.wikipedia.org/w/index.php?title=Operator-precedence_parser&diffonly=true#Pratt_parsing
 * https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
 * 
 * 메모리 누수는 신경쓰지 않고 작성한 코드니 주의.
 */

struct Expr;
class Parser;

using PrefixParseFn = std::function<Expr*(Parser*)>;
using InfixParseFn = std::function<Expr*(Parser*, Expr*)>;

struct Token
{
    enum class TokenType type;

    string lexeme;
};

enum class TokenType
{
    Char,

    Plus,
    Minus,
    Star,
    Slash,

    End,
};

enum class Precedence
{
    Lowest,
    Additive,
    Multiplicative,
    Primary,
    Highest,
};

int GetPrecedence(TokenType type)
{
    static std::map<TokenType, Precedence> typePrecedenceMap
    {
        { TokenType::Char, Precedence::Primary },
        { TokenType::Plus, Precedence::Additive },
        { TokenType::Minus, Precedence::Additive },
        { TokenType::Star, Precedence::Multiplicative },
        { TokenType::Slash, Precedence::Multiplicative },
        { TokenType::End, Precedence::Highest }
    };

    auto iter = typePrecedenceMap.find(type);

    return (int)(iter != typePrecedenceMap.end() ? iter->second : Precedence::Lowest);
}

struct Expr
{
    virtual ~Expr() = default;

    virtual void Print(int depth)
    {
        for (int i = 0; i < depth; i++)
        {
            std::cout << "    ";
        }
    }
};

struct CharExpr : Expr
{
    char ch;

    CharExpr(char ch)
        : ch{ ch }
    { }

    void Print(int depth) override
    {
        Expr::Print(depth);

        std::cout << "CharExpr[" << ch << "]\n";
    }
};

struct BinaryExpr : Expr
{
    char op;

    Expr* left;
    Expr* right;

    BinaryExpr(char op, Expr* left, Expr* right)
        : op{ op }, left{ left }, right{ right }
    { }

    void Print(int depth) override
    {
        Expr::Print(depth);

        std::cout << "BinaryExpr[" << op << "]\n";

        left->Print(depth + 1);
        right->Print(depth + 1);
    }
};

class Parser
{
public:
    Parser(std::vector<Token>& tokens)
        : _tokens{ tokens }, _tokenIdx{ 0 }
    { }

public:
    Expr* ParseExpression(Precedence precedence)
    {
        auto prefixFn = GetPrefixFn(CurrToken().type);

        if (nullptr == prefixFn)
        {
            std::cerr << "No Prefix Fn : " << CurrToken().lexeme << '\n';

            return nullptr;
        }

        Expr* left = prefixFn(this);

        // while ((int)precedence <= (int)GetPrecedence(NextToken().type)) // 이걸로 AST를 계산해보니 이건 문제가 있음(Bad Working).
        while ((int)precedence < (int)GetPrecedence(NextToken().type)) // 이걸 쓰면 문제 없음(Good Working).
        {
            auto infixFn = GetInfixFn(NextToken().type);

            if (nullptr == infixFn)
                return left;

            Advance();

            left = infixFn(this, left);
        }

        return left;
    }

public:
    void RegisterPrefixFn(TokenType type, PrefixParseFn fn)
    {
        _prefixParseFns[type] = fn;
    }

    void RegisterInfixFn(TokenType type, InfixParseFn fn)
    {
        _infixParseFns[type] = fn;
    }

    PrefixParseFn GetPrefixFn(TokenType type)
    {
        // 없으면 nullptr이 반환될 것임.
        return _prefixParseFns[type];
    }

    InfixParseFn GetInfixFn(TokenType type)
    {
        // 없으면 nullptr이 반환될 것임.
        return _infixParseFns[type];
    }

public:
    Token CurrToken()
    {
        return _tokens[_tokenIdx];
    }

    Token NextToken()
    {
        return _tokens[_tokenIdx + 1];
    }

    void Advance()
    {
        _tokenIdx++;
    }

private:
    std::vector<Token> _tokens;
    int _tokenIdx;

    std::map<TokenType, PrefixParseFn> _prefixParseFns;
    std::map<TokenType, InfixParseFn>  _infixParseFns;
};

int main()
{
    std::vector<Token> tokens;

    // A + B - C
    // tokens = { { TokenType::Char, "A" }, { TokenType::Plus, "+" }, { TokenType::Char, "B" }, { TokenType::Minus, "-" }, { TokenType::Char, "C" }, { TokenType::End, "" } };

    // A + B * C
    // tokens = { { TokenType::Char, "A" }, { TokenType::Plus, "+" }, { TokenType::Char, "B" }, { TokenType::Star, "*" }, { TokenType::Char, "C" }, { TokenType::End, "" } };

    // A * B + C
    // tokens = { { TokenType::Char, "A" }, { TokenType::Star, "*" }, { TokenType::Char, "B" }, { TokenType::Plus, "+" }, { TokenType::Char, "C" }, { TokenType::End, "" } };

    // A + B - C * D + E
    tokens = { { TokenType::Char, "A" }, { TokenType::Plus, "+" }, { TokenType::Char, "B" }, { TokenType::Minus, "-" }, { TokenType::Char, "C" }, { TokenType::Star, "*" }, { TokenType::Char, "D" }, { TokenType::Plus, "+" }, { TokenType::Char, "E" }, { TokenType::End, "" } };

    Parser parser{ tokens };

    // Test Code
    // auto fn = parser.GetInfixFn(TokenType::Char);
    // 
    // if (nullptr == fn)
    // {
    //     std::cout << "Empty\n";
    // }

    parser.RegisterPrefixFn(TokenType::Char, [](Parser* parser)
    {

        return new CharExpr{ parser->CurrToken().lexeme[0] };
    });

    parser.RegisterInfixFn(TokenType::Plus, [](Parser* parser, Expr* left)
    {
        parser->Advance(); // '+'

        Expr* right = parser->ParseExpression(Precedence::Additive);

        return new BinaryExpr{ '+', left, right};
    });
    
    parser.RegisterInfixFn(TokenType::Minus, [](Parser* parser, Expr* left)
    {
        parser->Advance(); // '-'

        Expr* right = parser->ParseExpression(Precedence::Additive);

        return new BinaryExpr{ '-', left, right};
    });
    
    parser.RegisterInfixFn(TokenType::Star, [](Parser* parser, Expr* left)
    {
        parser->Advance(); // '*'

        Expr* right = parser->ParseExpression(Precedence::Multiplicative);

        return new BinaryExpr{ '*', left, right};
    });
    
    parser.RegisterInfixFn(TokenType::Slash, [](Parser* parser, Expr* left)
    {
        parser->Advance(); // '/'

        Expr* right = parser->ParseExpression(Precedence::Multiplicative);
                       
        return new BinaryExpr{ '/', left, right};
    });

    Expr* root = parser.ParseExpression(Precedence::Lowest);

    root->Print(0);

    return 0;
}
