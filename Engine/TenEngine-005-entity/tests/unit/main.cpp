/**
 * @file main.cpp
 * @brief Test main entry point for Entity module
 */

#include <cassert>
#include <iostream>

// Test function declarations
extern int test_entity();
extern int test_component();
extern int test_component_query();
extern int test_entity_manager();

int main() {
    std::cout << "Running Entity module tests..." << std::endl;
    
    int result = 0;
    
    std::cout << "  Testing Entity..." << std::endl;
    result |= test_entity();
    
    std::cout << "  Testing Component..." << std::endl;
    result |= test_component();
    
    std::cout << "  Testing ComponentQuery..." << std::endl;
    result |= test_component_query();
    
    std::cout << "  Testing EntityManager..." << std::endl;
    result |= test_entity_manager();
    
    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << "Some tests failed!" << std::endl;
    }
    
    return result;
}
