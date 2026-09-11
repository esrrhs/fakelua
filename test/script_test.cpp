#include "fk_test.h"

#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace {

bool ends_with_fk(const char *name) {
    size_t n = strlen(name);
    return n >= 3 && strcmp(name + n - 3, ".fk") == 0;
}

void list_scripts_rec(const std::string &root, const std::string &rel, std::vector<std::string> *out) {
    std::string path = rel.empty() ? root : root + "/" + rel;
    DIR *d = opendir(path.c_str());
    if (!d) {
        return;
    }
    while (dirent *e = readdir(d)) {
        const char *name = e->d_name;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }
        if (strcmp(name, "package") == 0) {
            continue;
        }
        std::string child_rel = rel.empty() ? name : rel + "/" + name;
        std::string child_path = root + "/" + child_rel;
        struct stat st;
        if (stat(child_path.c_str(), &st) != 0) {
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            list_scripts_rec(root, child_rel, out);
        } else if (S_ISREG(st.st_mode) && ends_with_fk(name)) {
            out->push_back(child_rel);
        }
    }
    closedir(d);
}

std::vector<std::string> list_scripts(const char *dir) {
    std::vector<std::string> out;
    list_scripts_rec(dir, "", &out);
    std::sort(out.begin(), out.end());
    return out;
}

std::string dirname_of(const std::string &rel) {
    size_t p = rel.find_last_of('/');
    if (p == std::string::npos) {
        return ".";
    }
    return rel.substr(0, p);
}

std::string basename_of(const std::string &rel) {
    size_t p = rel.find_last_of('/');
    if (p == std::string::npos) {
        return rel;
    }
    return rel.substr(p + 1);
}

}  // namespace

class ScriptTest : public ::testing::TestWithParam<std::string> {
protected:
    fake *fk = nullptr;
    static std::string root;
    static std::string cwd0;

    static void SetUpTestSuite() {
        root = FAKE_SCRIPT_DIR;
        char buf[PATH_MAX];
        ASSERT_NE(getcwd(buf, sizeof(buf)), nullptr);
        cwd0 = buf;
    }

    static void TearDownTestSuite() {
        if (!cwd0.empty()) {
            chdir(cwd0.c_str());
        }
    }

    void SetUp() override {
        fk = fk_test_new();
        ASSERT_NE(fk, nullptr);
    }

    void TearDown() override {
        if (fk) {
            delfake(fk);
            fk = nullptr;
        }
        if (!cwd0.empty()) {
            chdir(cwd0.c_str());
        }
    }
};

std::string ScriptTest::root;
std::string ScriptTest::cwd0;

TEST_P(ScriptTest, RunsMain) {
    const std::string &rel = GetParam();
    std::string dir = root + "/" + dirname_of(rel);
    std::string base = basename_of(rel);
    ASSERT_EQ(chdir(dir.c_str()), 0) << dir;
    ASSERT_TRUE(fkparse(fk, base.c_str())) << rel << ": " << fkerrorstr(fk);
    fkrun<int>(fk, "main");
    EXPECT_EQ(fkerror(fk), efk_ok) << rel << ": " << fkerrorstr(fk);
}

INSTANTIATE_TEST_SUITE_P(
        Script,
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
