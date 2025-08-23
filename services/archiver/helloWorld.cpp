#include <iostream>

int main() {
    // Verify that we are using at least C++23
    static_assert(__cplusplus >= 202100L, 
                  "This program requires C++23 or later");

    std::cout << __cplusplus;
    return 0;
}
