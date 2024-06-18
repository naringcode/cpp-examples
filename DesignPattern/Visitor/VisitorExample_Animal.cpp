#include <iostream>

/**
 * Visitor 패턴 예시 : 동물들의 속성과 행동 관리
 */

// Visitor 인터페이스
class AnimalVisitor
{
public:
    virtual ~AnimalVisitor() = default;

public:
    virtual void visitDog(class Dog* dog) = 0;
    virtual void visitCat(class Cat* cat) = 0;
};

// Element 인터페이스
class AnimalElement
{
public:
    virtual ~AnimalElement() = default;

public:
    virtual void accept(class AnimalVisitor* visitor) = 0;

    virtual void someAction() = 0;
};

// Dog 구현
class Dog : public AnimalElement
{
public:
    void accept(AnimalVisitor* visitor) override
    {
        visitor->visitDog(this);
    }

    void someAction() override
    {
        std::cout << "Dog is fetching a ball.\n";
    }

    void bark()
    {
        std::cout << "Dog barks!\n";
    }
};

// Cat 구현
class Cat : public AnimalElement
{
public:
    void accept(AnimalVisitor* visitor) override
    {
        visitor->visitCat(this);
    }

    void someAction() override
    {
        std::cout << "Cat is playing with a yarn.\n";
    }

    void meow()
    {
        std::cout << "Cat meows!\n";
    }
};

// Visitor 구현(1)
class ActionVisitor : public AnimalVisitor
{
public:
    void visitDog(Dog* dog) override
    {
        std::cout << "Dog starts doing something...\n";

        dog->someAction();
    }

    void visitCat(Cat* cat) override
    {
        std::cout << "Cat starts doing something...\n";

        cat->someAction();
    }
};

// Visitor 구현(2)
class SoundVisitor : public AnimalVisitor
{
public:
    void visitDog(Dog* dog) override
    {
        dog->bark();
    }

    void visitCat(Cat* cat) override
    {
        cat->meow();
    }
};

int main() 
{
    Dog dog;
    Cat cat;

    ActionVisitor actionVisitor;
    SoundVisitor  soundVisitor;

    dog.accept(&actionVisitor);
    cat.accept(&actionVisitor);

    std::cout << "\n";

    dog.accept(&soundVisitor);
    cat.accept(&soundVisitor);

    return 0;
}
