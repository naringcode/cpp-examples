// Global Module Fragment : Optional
module;

#include <iostream>
#include <concepts>

// Module Preamble : Required
export module BasicModule; // 모듈 이름

// Module Purview / Module Interface : Optional

// export를 붙이지 않은 엔터티는 모듈 내에서만 보여진다.
void ModuleLinkageFunc()
{
    std::cout << "InnerFunc()\n";
}

// export를 붙여야 해당 모듈을 포함하는 쪽에서 접근하여 사용할 수 있다.
export void ExternalLinkageFunc()
{
    std::cout << "ExternalLinkageFunc()\n";

    ModuleLinkageFunc();
}

// 템플릿이 적용된 것도 export 가능하다.
/* export */ template <typename T>
concept ArithmeticConcept = std::integral<T> || std::floating_point<T>;

// export 블록 안에 있는 엔터티는 export가 적용된 것으로 간주한다.
export
{
    void ExternalLinkageFuncInExportBlock()
    {
        std::cout << "ExternalLinkageFuncInExportBlock()\n";
    }

    // ArithmeticConcept은 모듈 내에서만 사용하게 제한했지만 실제 사용할 때는 별 문제 없다.
    // 이건 가시성(Visibility)과 도달성(Reachability)에 대한 차이이다.
    // ArithmeticConcept은 사용하는 쪽에 공개되어 있지 않지만 컴파일러가 인자를 추론하여 타입을 유추할 수 있다.
    template <ArithmeticConcept T1, ArithmeticConcept T2>
    std::common_type_t<T1, T2> Add(T1 x, T2 y)
    {
        std::cout << typeid(x + y).name()
            << " Add(" 
            << typeid(x).name() << ", " 
            << typeid(T2).name() << ")\n";

        return x + y;
    }

    struct Point
    {
        int x;
        int y;
    };

    void PrintPoint(const Point& p);
}

// export 엔터티를 선언하고 이렇게 나중에 구현하는 것도 가능하다.
void PrintPoint(const Point& p)
{
    std::cout << "[" << p.x << ", " << p.y << "]\n";
}

// Private Module Fragment : Optional
module: private;
