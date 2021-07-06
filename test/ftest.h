#pragma once

#include <string>
#include <experimental/source_location>
#include <filesystem>
#include <iostream>

class FTestCaseBase {
public:
    FTestCaseBase(const std::string &pkg_name, const std::string &testcase_name,
                  const std::experimental::source_location &location) {
        m_pkg_name = pkg_name;
        m_testcase_name = testcase_name;
        m_location = location;
    }

    void Fatal(const std::experimental::source_location &location, const std::string &oper, const std::string &expr,
               const std::string &l, const std::string &r) {
        std::stringstream ss;
        ss << "Package:" << m_pkg_name << ", Case:" << m_testcase_name
           << " FilePos:" << std::filesystem::path(m_location.file_name()).filename().string() << ":"
           << m_location.line() << std::endl
           << "\tAssertPos:" << std::filesystem::path(location.file_name()).filename().string() << ":"
           << location.line()
           << "\t\tFailed: " << expr << oper << l << " but get " << r << std::endl;
        std::cout << (ss.str());
        exit(-1);
    }

    void Ok(const std::experimental::source_location &location, const std::string &oper, const std::string &expr,
            const std::string &r) {
        std::stringstream ss;
        ss << "Package:" << m_pkg_name << ", Case:" << m_testcase_name
           << " Ok: " << expr << oper << r << std::endl;
        std::cout << (ss.str());
    }

private:
    std::string m_pkg_name;
    std::string m_testcase_name;
    std::experimental::source_location m_location;
};

#define TEST(pkg, testcase) \
    class F_##pkg##_##testcase##_TestCase : public FTestCaseBase { \
    public: \
        F_##pkg##_##testcase##_TestCase(const std::string &pkg_name, const std::string &testcase_name, \
                  const std::experimental::source_location &location) : FTestCaseBase(pkg_name, testcase_name, location) {} \
        ~F_##pkg##_##testcase##_TestCase() { Run(); } \
     \
        void Run(); \
     \
    }; \
    F_##pkg##_##testcase##_TestCase gF_##pkg##_##testcase##_TestCase(#pkg, #testcase, std::experimental::source_location::current()); \
    void F_##pkg##_##testcase##_TestCase::Run()

template<typename T>
std::string ftostring(const T &t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

#define ASSERT_OPER(left, right, oper) \
    { \
        auto l = left; \
        auto r = right; \
        if (!(l oper r)) { \
            this->Fatal(std::experimental::source_location::current(), #oper, #left,ftostring(l), ftostring(r)); \
        } else {                    \
            this->Ok(std::experimental::source_location::current(), #oper, #left,ftostring(r)); \
        } \
    }

#define ASSERT_NE(left, right) ASSERT_OPER(left, right, !=)

#define ASSERT_EQ(left, right) ASSERT_OPER(left, right, ==)
