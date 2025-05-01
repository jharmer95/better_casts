#include "better_casts.hpp"

#include <cstdint>
#include <iostream>

int main()
{
    try
    {
        auto x = casts::narrow_cast_checked<int8_t>(6'666'666);

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
