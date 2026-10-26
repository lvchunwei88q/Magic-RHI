// Test.h
// The test database is used for some functional testing
#include <iostream>
#include <vector>
#include <string>

inline int& GetFailedCount() { static int count = 0; return count; }

// Is it true?
#define EXPECT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "FAILED: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it false?
#define EXPECT_FALSE(expr) \
    do { \
        if (expr) { \
            std::cerr << "FAILED: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it equal?
#define EXPECT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            std::cerr << "FAILED: " << #a << " != " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it not equal?
#define EXPECT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            std::cerr << "FAILED: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
        } \
    } while (0)
// Is it true? If not, exit the program.
#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::cerr << "FATAL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)
// Is it false? If not, exit the program.
#define ASSERT_FALSE(expr) \
    do { \
        if (expr) { \
            std::cerr << "FATAL: " << #expr << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)
// Is it equal? If not, exit the program.
#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            std::cerr << "FATAL: " << #a << " != " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)
// Is it not equal? If not, exit the program.
#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            std::cerr << "FATAL: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            GetFailedCount()++; \
            std::exit(1); \
        } \
    } while (0)
// main function for running tests
inline int RunTests(void (*testFunc)()) 
{
    GetFailedCount() = 0;
    testFunc();
    int failedCount = GetFailedCount();
    if (failedCount > 0) {
        std::cerr << "Tests failed: " << failedCount << std::endl;
        return 1;
    } else {
        std::cout << "All tests passed!" << std::endl;
        return 0;
    }
}
// define main function
#define TEST_MAIN(...) \
    int main() { \
        std::vector<void (*)()> funcs = { __VA_ARGS__ }; \
        size_t total = funcs.size(); \
        int totalFailed = 0; \
        \
        for (size_t i = 0; i < total; ++i) { \
            std::cout << "[" << (i + 1) << "/" << total << "] Running test..." << std::endl; \
            int failed = RunTests(funcs[i]); \
            if (failed > 0) { \
                std::cerr << "  -> Failed: " << failed << " error(s).\n"; \
                totalFailed += failed; \
            } else { \
                std::cout << "  -> Passed.\n"; \
            } \
        } \
        \
        std::cout << "\n==============================\n"; \
        if (totalFailed == 0) { \
            std::cout << "All " << total << " test(s) passed!\n"; \
            return 0; \
        } else { \
            std::cerr << "Total failures: " << totalFailed << "\n"; \
            return 1; \
        } \
    }