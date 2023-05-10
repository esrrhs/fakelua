#include "fakelua.h"
#include "util/concurrent_hashmap.h"
#include "util/concurrent_vector.h"
#include "gtest/gtest.h"

using namespace fakelua;

TEST(common, concurrent_hashmap_int) {
    concurrent_hashmap<int, int> map1(2);
    int v;
    auto ret = map1.get(1, v);
    ASSERT_EQ(ret, false);

    map1.set(1, 2);
    auto size = map1.size();
    ASSERT_EQ(size, 1);

    ret = map1.check_rehash();
    ASSERT_EQ(ret, false);

    ret = map1.get(1, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 2);

    map1.set(1, 3);
    ret = map1.get(1, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 3);
    size = map1.size();
    ASSERT_EQ(size, 1);

    map1.set(2, 4);
    size = map1.size();
    ASSERT_EQ(size, 2);
    ret = map1.get(2, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 4);

    map1.set(3, 5);
    size = map1.size();
    ASSERT_EQ(size, 3);
    ret = map1.get(3, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 5);

    map1.remove(1);
    size = map1.size();
    ASSERT_EQ(size, 2);

    ret = map1.check_rehash();
    ASSERT_EQ(ret, true);

    map1.set(4, 6);
    size = map1.size();
    ASSERT_EQ(size, 3);
    ret = map1.get(4, v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, 6);
}

TEST(common, concurrent_hashmap_string) {
    concurrent_hashmap<std::string, std::string> map1(2);
    std::string v;
    auto ret = map1.get("1", v);
    ASSERT_EQ(ret, false);

    map1.set("1", "2");
    auto size = map1.size();
    ASSERT_EQ(size, 1);

    ret = map1.check_rehash();
    ASSERT_EQ(ret, false);

    ret = map1.get("1", v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, "2");

    map1.set("1", "3");
    ret = map1.get("1", v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, "3");

    map1.set("2", "4");
    size = map1.size();
    ASSERT_EQ(size, 2);

    ret = map1.check_rehash();
    ASSERT_EQ(ret, true);

    map1.set("3", "5");
    size = map1.size();
    ASSERT_EQ(size, 3);
    ret = map1.get("3", v);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v, "5");

    map1.remove("1");
    size = map1.size();
    ASSERT_EQ(size, 2);
}

TEST(common, concurrent_vector_int) {
    concurrent_vector<int> v(2);
    int v1;
    auto ret = v.get(1, v1);
    ASSERT_EQ(ret, false);
    auto size = v.size();
    ASSERT_EQ(size, 0);

    v.set(1, 2);
    ret = v.get(1, v1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v1, 2);
    size = v.size();
    ASSERT_EQ(size, 1);

    v.set(2, 3);
    ret = v.get(2, v1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v1, 3);
    size = v.size();
    ASSERT_EQ(size, 2);

    v.set(3, 4);
    ret = v.get(3, v1);
    ASSERT_EQ(ret, true);
    ASSERT_EQ(v1, 4);
    size = v.size();
    ASSERT_EQ(size, 3);

    v.remove(1);
    ret = v.get(1, v1);
    ASSERT_EQ(ret, false);
    size = v.size();
    ASSERT_EQ(size, 2);
}

TEST(common, concurrent_hashmap_multi_thread) {
    concurrent_hashmap<int, int> map1(100);
    auto t1 = std::thread([&map1]() {
        for (int i = 0; i < 100000; ++i) {
            map1.set(i, i);
        }
    });
    auto t2 = std::thread([&map1]() {
        for (int i = 0; i < 100000; ++i) {
            map1.set(i, i);
        }
    });
    t1.join();
    t2.join();
    for (int i = 0; i < 100000; ++i) {
        int v;
        auto ret = map1.get(i, v);
        ASSERT_EQ(ret, true);
        ASSERT_EQ(v, i);
    }

    t1 = std::thread([&map1]() {
        for (int i = 0; i < 50000; ++i) {
            map1.set(i, i);
        }
    });

    t2 = std::thread([&map1]() {
        for (int i = 50000; i < 100000; ++i) {
            map1.remove(i);
        }
    });

    t1.join();
    t2.join();

    for (int i = 0; i < 50000; ++i) {
        int v;
        auto ret = map1.get(i, v);
        ASSERT_EQ(ret, true);
        ASSERT_EQ(v, i);
    }
}