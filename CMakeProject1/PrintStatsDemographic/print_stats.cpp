/*
#include <string>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <functional>
#include <iostream>

using namespace std;

enum class Gender {
    FEMALE,
    MALE
};

struct Person {
    int age;  // возраст
    Gender gender;  // пол
    bool is_employed;  // имеет ли работу
};

// Это пример функции, его не нужно отправлять вместе с функцией PrintStats
template <typename InputIt>
int ComputeMedianAge(InputIt range_begin, InputIt range_end) {
    if (range_begin == range_end) {
        return 0;
    }
    vector<typename InputIt::value_type> range_copy(range_begin, range_end);
    auto middle = begin(range_copy) + range_copy.size() / 2;
    nth_element(
        begin(range_copy), middle, end(range_copy),
        [](const Person& lhs, const Person& rhs) {
            return lhs.age < rhs.age;
        }
    );
    return middle->age;
}

*/
#include <functional>

void PrintStats(vector<Person> persons) {
    vector<string> output = {
        "Median age = ",
        "Median age for females = ",
        "Median age for males = ",
        "Median age for employed females = ",
        "Median age for unemployed females = ",
        "Median age for employed males = ",
        "Median age for unemployed males = "
    };

    vector<function<bool(const Person&)>> lambdas{
        [](const Person& p) {return true; },
        [](const Person& p) {return p.gender == Gender::FEMALE; },
        [](const Person& p) {return p.gender == Gender::MALE; },
        [](const Person& p) {return p.gender == Gender::FEMALE && p.is_employed; },
        [](const Person& p) {return p.gender == Gender::FEMALE && !p.is_employed; },
        [](const Person& p) {return p.gender == Gender::MALE && p.is_employed; },
        [](const Person& p) {return p.gender == Gender::MALE && !p.is_employed; }
    };

    for (size_t i = 0; i < output.size(); ++i) {
        auto it = partition(persons.begin(), persons.end(), lambdas[i]);
        cout << output[i] << ComputeMedianAge(persons.begin(), it) << "\n";
    }
}
/*
int main() {
    vector<Person> persons = {
        {31, Gender::MALE, false},
        {40, Gender::FEMALE, true},
        {24, Gender::MALE, true},
        {20, Gender::FEMALE, true},
        {80, Gender::FEMALE, false},
        {78, Gender::MALE, false},
        {10, Gender::FEMALE, false},
        {55, Gender::MALE, true},
    };
    PrintStats(persons);
    return 0;
}
*/
