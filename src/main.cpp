#include "main.h"
#include <iostream>
#include <string>
#include "PoolAllocator.h"
#include "StackAllocator.h"
//#include "GeneralPurposeAllocator.h"

class Person
{
public:
    int m_age;
    std::string m_name;
    std::string m_haircolor;

    Person(int age, std::string name, std::string haircolor)
        : m_age(age), m_name(name), m_haircolor(haircolor)
    {
    }

    void print()
    {
        std::cout << m_name << " " << m_age << " " << m_haircolor << std::endl;
    }
};

int main()
{
    std::cout << "GameEngine project!" << std::endl;


    return EXIT_SUCCESS;
}