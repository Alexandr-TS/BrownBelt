#include "test_runner.h"
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <algorithm>
#include <set>
#include <utility>
#include <vector>

using namespace std;

template <typename T>
class PriorityCollection {
public:
    using Id = int;

    // Добавить объект с нулевым приоритетом
    // с помощью перемещения и вернуть его идентификатор
    Id Add(T object) {
        objects.emplace_back(move(object));
        priority.emplace_back(0);
        priors.insert(make_pair(0, current_id));
        return current_id++;
    }

    // Добавить все элементы диапазона [range_begin, range_end)
    // с помощью перемещения, записав выданные им идентификаторы
    // в диапазон [ids_begin, ...)
    template <typename ObjInputIt, typename IdOutputIt>
    void Add(ObjInputIt range_begin, ObjInputIt range_end,
        IdOutputIt ids_begin) {
        for (auto it = range_begin; it != range_end; ++it) {
            *ids_begin = Add(move(*it));
            ids_begin++;
        }
    }

    // Определить, принадлежит ли идентификатор какому-либо
    // хранящемуся в контейнере объекту
    bool IsValid(Id id) const {
        if (id < 0 || id >= static_cast<Id>(priority.size())) {
            return false;
        }
        return priority[id] >= 0;
    }

    // Получить объект по идентификатору
    const T& Get(Id id) const {
        return objects[id];
    }

    // Увеличить приоритет объекта на 1
    void Promote(Id id) {
        priors.erase(make_pair(priority[id], id));
        ++priority[id];
        priors.insert(make_pair(priority[id], id));
    }

    // Получить объект с максимальным приоритетом и его приоритет
    pair<const T&, int> GetMax() const {
        int id = priors.begin()->second;
        return { (objects[id]), priority[id] };
    }

    // Аналогично GetMax, но удаляет элемент из контейнера
    pair<T, int> PopMax() {
        int id = priors.begin()->second;
        priors.erase(priors.begin());
        int tmp_p = priority[id];
        priority[id] = -1;
        return { move(objects[id]), tmp_p };
    }

private:
    Id current_id = 0;
    vector<T> objects;
    vector<Id> priority;
    set <pair<int, Id>, greater<pair<int, Id>>> priors;
};


class StringNonCopyable : public string {
public:
    using string::string;  // Позволяет использовать конструкторы строки
    StringNonCopyable(const StringNonCopyable&) = delete;
    StringNonCopyable(StringNonCopyable&&) = default;
    StringNonCopyable& operator=(const StringNonCopyable&) = delete;
    StringNonCopyable& operator=(StringNonCopyable&&) = default;
};

void TestNoCopy() {
    PriorityCollection<StringNonCopyable> strings;
    const auto white_id = strings.Add("white");
    const auto yellow_id = strings.Add("yellow");
    const auto red_id = strings.Add("red");

    strings.Promote(yellow_id);
    for (int i = 0; i < 2; ++i) {
        strings.Promote(red_id);
    }
    strings.Promote(yellow_id);
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "red");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "yellow");
        ASSERT_EQUAL(item.second, 2);
    }
    {
        const auto item = strings.PopMax();
        ASSERT_EQUAL(item.first, "white");
        ASSERT_EQUAL(item.second, 0);
    }
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestNoCopy);
    return 0;
}