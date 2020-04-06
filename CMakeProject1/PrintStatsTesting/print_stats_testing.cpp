#include "test_runner.h"
//#include "solution_b.h"
#include <vector>
#include <sstream>
#include <memory>
#include <algorithm>
#include <random>

using namespace std;

void TestOperatorEqual() {
    //ASSERT(record != nullptr);
    //ASSERT_EQUAL(final_body, record->title);
    Person p{ 12, Gender::MALE, true };
    Person p_same{ 12, Gender::MALE, true };
    Person p1{ 13, Gender::MALE, true };
    Person p2{ 12, Gender::FEMALE, true };
    Person p3{ 12, Gender::MALE, false};
    ASSERT(p == p_same);
    ASSERT(!(p == p1));
    ASSERT(!(p == p2));
    ASSERT(!(p == p3));
}

void TestComputeMedianAge() {
    mt19937 gen;
    for (int n : {2, 9, 10, 11}) {
        vector<Person> v;
        for (int i = 0; i < n; ++i) {
            v.push_back(Person{ i, Gender::MALE, true });
        }
        shuffle(v.begin(), v.end(), gen);
        auto res = ComputeMedianAge(v.begin(), v.end());
        int exp = n / 2;
        ASSERT_EQUAL(res, exp);
    }

    vector<Person> v2;
    auto res = ComputeMedianAge(v2.begin(), v2.end());
    int expected = 0;
    ASSERT_EQUAL(expected, res);
}

void TestRead() {
    stringstream ss;
    int n = 5;
    ss << n << " ";
    mt19937 gen;
    vector<Person> v;
    for (int i = 0; i < n; ++i) {
        int gender = static_cast<int>(gen() % 2u);
        int employed = static_cast<int>(gen() % 2u);
        Person p{ i + static_cast<int>(gen() % 20u),
            static_cast<Gender>(gender), static_cast<bool>(employed) };
        v.push_back(p);
        ss << p.age << " " << gender << " " << employed << "\n";
    }
    auto ret = ReadPersons(ss);
    ASSERT_EQUAL(ret, v);
}

void TestComputeStats() {
    vector<Person> v = {
        Person{10, Gender::MALE, true},
        Person{12, Gender::MALE, false},
        Person{14, Gender::FEMALE, true},
        Person{16, Gender::FEMALE, false},
        Person{18, Gender::FEMALE, true},
        Person{20, Gender::MALE, false},
        Person{20, Gender::MALE, false}
    };
    auto ret = ComputeStats(v);
    //AgeStats st{ 7, 3, 4, 2, 1, 1, 3 };
    AgeStats st{ 16, 16, 20, 18, 16, 10, 20 };
    ASSERT_EQUAL(ret.total, st.total);
    ASSERT_EQUAL(ret.females, st.females);
    ASSERT_EQUAL(ret.males, st.males);
    ASSERT_EQUAL(ret.employed_females, st.employed_females);
    ASSERT_EQUAL(ret.unemployed_females, st.unemployed_females);
    ASSERT_EQUAL(ret.employed_males, st.employed_males);
    ASSERT_EQUAL(ret.unemployed_males, st.unemployed_males);

    vector<Person> v2 = {
        Person{34, Gender::FEMALE, false}
    };
    ret = ComputeStats(v2);
    st = AgeStats{ 34, 34, 0, 0, 34, 0, 0 };
    ASSERT_EQUAL(ret.total, st.total);
    ASSERT_EQUAL(ret.females, st.females);
    ASSERT_EQUAL(ret.males, st.males);
    ASSERT_EQUAL(ret.employed_females, st.employed_females);
    ASSERT_EQUAL(ret.unemployed_females, st.unemployed_females);
    ASSERT_EQUAL(ret.employed_males, st.employed_males);
    ASSERT_EQUAL(ret.unemployed_males, st.unemployed_males);
}

void TestPrintStats() {
    stringstream ss;
    AgeStats st{ 7, 3, 4, 2, 1, 1, 3 };
    PrintStats(st, ss);
    string s;
    vector<string> expected {
        "Median age = 7",
        "Median age for females = 3",
        "Median age for males = 4",
        "Median age for employed females = 2",
        "Median age for unemployed females = 1",
        "Median age for employed males = 1",
        "Median age for unemployed males = 3"
    };
    for (int i = 0; i < 7; ++i) {
        getline(ss, s);
        string exp = expected[i];
        ASSERT_EQUAL(s, exp);
    }
}

void TestOperatorPrint() {
    {
        Person p{ 10, Gender::MALE, true };
        stringstream ss;
        ss << p;
        string s;
        getline(ss, s);
        string expected = "Person(age=10, gender=1, is_employed=true)";
        ASSERT_EQUAL(s, expected);
    }
    {
        Person p{ 41, Gender::FEMALE, false};
        stringstream ss;
        ss << p;
        string s;
        getline(ss, s);
        string expected = "Person(age=41, gender=0, is_employed=false)";
        ASSERT_EQUAL(s, expected);
    }
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestOperatorEqual);
    RUN_TEST(tr, TestOperatorPrint);
    RUN_TEST(tr, TestComputeMedianAge);
    RUN_TEST(tr, TestRead);
    RUN_TEST(tr, TestComputeStats);
    RUN_TEST(tr, TestPrintStats);
}
