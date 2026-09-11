#include "fk_test.h"

#include <cctype>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace {

std::vector<std::string> list_scripts(const char *dir) {
    std::vector<std::string> out;
    DIR *d = opendir(dir);
    if (!d) {
        return out;
    }
    while (dirent *e = readdir(d)) {
        const char *name = e->d_name;
        size_t n = strlen(name);
        if (n < 4 || strcmp(name + n - 3, ".fk") != 0) {
            continue;
        }
        out.push_back(name);
    }
    closedir(d);
    return out;
}

}  // namespace

class ScriptTest : public ::testing::TestWithParam<std::string> {
protected:
    static fake *fk;

    static void SetUpTestSuite() {
        ASSERT_EQ(chdir(FAKE_SCRIPT_DIR), 0) << FAKE_SCRIPT_DIR;
        fk = fk_test_new();
        ASSERT_NE(fk, nullptr);
    }

    static void TearDownTestSuite() {
        if (fk) {
            delfake(fk);
            fk = nullptr;
        }
    }

    void SetUp() override {
        fkreset(fk);
        fkclear(fk);
    }
};

fake *ScriptTest::fk = nullptr;

TEST_P(ScriptTest, RunsMain) {
    const std::string &name = GetParam();
    ASSERT_TRUE(fkparse(fk, name.c_str())) << name << ": " << fkerrorstr(fk);
    fkrun<int>(fk, "main");
    EXPECT_EQ(fkerror(fk), efk_ok) << name << ": " << fkerrorstr(fk);
}

INSTANTIATE_TEST_SUITE_P(
        Sample,
        ScriptTest,
        ::testing::ValuesIn(list_scripts(FAKE_SCRIPT_DIR)),
        [](const ::testing::TestParamInfo<std::string> &info) {
            std::string n = info.param;
            for (char &c : n) {
                if (!std::isalnum(static_cast<unsigned char>(c))) {
                    c = '_';
                }
            }
            return n;
        });
