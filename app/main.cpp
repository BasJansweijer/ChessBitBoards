#include "core.h"
#include <iostream>

int main()
{
    core_function();
    std::cout << "Sum: " << add(5, 3) << std::endl;
    std::cout << "Product: " << multiply(4.2, 2.0) << std::endl;
    return 0;
}
