#include "better_casts.hpp"

#include <cstdint>
#include <iostream>

namespace
{
void my_handler(const casts::cast_violation& violation)
{
    std::cerr << "MyHandler: " << violation.message << '\n';
}
} //namespace

int main()
{
    try
    {
        casts::set_violation_handler(my_handler);
        auto x = casts::narrow_cast_checked<int8_t>(6'666'666);

        casts::set_violation_handler();

        auto y = casts::sign_cast_checked<uint32_t>(-1);

        (void)x;
        (void)y;
    }
    catch (const casts::cast_error& ex)
    {
        std::cerr << "Exception: " << ex.what() << '\n';
    }

    std::cout << "Hello from C++ ver. " << __cplusplus << '\n';
}
