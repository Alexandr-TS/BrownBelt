#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include <optional>
#include <map>
#include <unordered_map>
#include <string>
#include <numeric>

using namespace std;

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end)
        : first(begin)
        , last(end)
    {
    }

    Iterator begin() const {
        return first;
    }

    Iterator end() const {
        return last;
    }

private:
    Iterator first, last;
};

template <typename Collection>
auto Head(Collection& v, size_t top) {
    return IteratorRange{ v.begin(), next(v.begin(), min(top, v.size())) };
}

struct Person {
    string name;
    int age, income;
    bool is_male;
};

vector<Person> ReadPeople(istream& input) {
    int count;
    input >> count;

    vector<Person> result(count);
    for (Person& p : result) {
        char gender;
        input >> p.name >> p.age >> p.income >> gender;
        p.is_male = gender == 'M';
    }

    return result;
}

struct CntByNameCache {
    string answer = "";
    bool valid = false;
    bool no_answer = false;
};

int main() {
    const vector<Person> people = ReadPeople(cin);
    const vector<Person> people_by_w = [&people]() {
        vector<Person> tmp = people;
		sort(begin(tmp), end(tmp), [](const Person& lhs, const Person& rhs) {
			return lhs.income > rhs.income;
			});
        return tmp;
    }();

    CntByNameCache male, female;
    vector<long long> pref_sum(people.size() + 1);
    for (size_t i = 0; i < people_by_w.size(); ++i) {
        pref_sum[i + 1] = pref_sum[i] + people_by_w[i].income;
    }

    for (string command; cin >> command; ) {
        if (command == "AGE") {
            int adult_age;
            cin >> adult_age;

            int answer = count_if(people.begin(), people.end(),
                [adult_age](const Person& pers) {
                    return pers.age >= adult_age; 
                });

            cout << "There are " << answer
                << " adult people for maturity age " << adult_age << '\n';
        }
        else if (command == "WEALTHY") {
            int count;
            cin >> count;
            count = min(count, (int)people_by_w.size());

            int total_income = pref_sum[count];
            cout << "Top-" << count << " people have total income " << total_income << '\n';
        }
        else if (command == "POPULAR_NAME") {
            char gender;
            cin >> gender;
            CntByNameCache* cache = nullptr;
            if (gender == 'W') {
                cache = &female;
            }
            else if (gender == 'M') {
                cache = &male;
            }
            else {
                assert(false);
            }
            if (cache->valid) {
                if (cache->no_answer) {
                    cout << "No people of gender " << gender << '\n';
                }
                else {
                    cout << "Most popular name among people of gender " << gender << " is ";
                    cout << cache->answer << "\n";
                }
                continue;
            }
            map<string, int> cnt_by_name;
            for (const auto& pers : people) {
                if ((pers.is_male && (gender == 'M')) || (!pers.is_male && (gender == 'W'))) {
                    cnt_by_name[pers.name]++;
                }
            }
            if (cnt_by_name.empty()) {
                cache->valid = true;
                cache->no_answer = true;
                cout << "No people of gender " << gender << '\n';
            }
            else {
				auto it = cnt_by_name.begin();
                for (auto it2 = cnt_by_name.begin(); it2 != cnt_by_name.end(); ++it2) {
                    if (it2->second > it->second || it->second == it2->second && it2->first < it->first) {
                        it = it2;
                    }
                }
                cout << "Most popular name among people of gender " << gender << " is "
                    << (it->first) << '\n';
                cache->valid = true;
                cache->no_answer = false;
                cache->answer = (it->first);
            }
        }
    }
}
