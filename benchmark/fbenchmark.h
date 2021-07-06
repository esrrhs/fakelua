#pragma once

#include <string>
#include <chrono>
#include <iostream>

class FBenchmarkCaseBase {
public:
    FBenchmarkCaseBase(const std::string &pkg_name, const std::string &benchmarkcase_name) {
        m_pkg_name = pkg_name;
        m_benchmarkcase_name = benchmarkcase_name;
    }

    virtual void Run() = 0;

    void RunAllBenchmark() {
        RunBenchmark(100);
        RunBenchmark(10000);
        RunBenchmark(1000000);
    }

private:
    void RunBenchmark(int times) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < times; ++i) {
            this->Run();
        }
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        std::cout << "Package:" << this->m_pkg_name << ", Case:" << this->m_benchmarkcase_name
                  << " Count:" << times << " Time:" << elapsed.count() << "ns"
                  << " PerRun:" << elapsed.count() / times << "ns" << std::endl;
    }

private:
    std::string m_pkg_name;
    std::string m_benchmarkcase_name;
};

extern void RunAllBenchmark();

#define FBENCHMARK(pkg, benchmarkcase) \
    class F_##pkg##_##benchmarkcase##_Case : public FBenchmarkCaseBase { \
    public: \
        F_##pkg##_##benchmarkcase##_Case(const std::string &pkg_name, const std::string &benchmarkcase_name) :FBenchmarkCaseBase(pkg_name,benchmarkcase_name) {} \
        ~F_##pkg##_##benchmarkcase##_Case() { RunAllBenchmark(); } \
     \
        virtual void Run() override; \
     \
    }; \
    F_##pkg##_##benchmarkcase##_Case gF_##pkg##_##benchmarkcase##_Case(#pkg, #benchmarkcase); \
    void F_##pkg##_##benchmarkcase##_Case::Run()
