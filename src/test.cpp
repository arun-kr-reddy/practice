#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    printf("hello\n");

    return 0;
}

class animalClass  // super class
{
public:
    animalClass(){};
    ~animalClass(){};
    void eat(){};
    void move(){};
    std::string name;
    uint32_t height;
    uint32_t weight;
};

class birdClass : public animalClass  // sub class 1
{
    void move() override{};  // override
};

class dogClass : public animalClass  // sub class 2
{
    void digHole() override{};  // extend
};