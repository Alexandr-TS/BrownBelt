#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <queue>
#include <stdexcept>
#include <set>
using namespace std;

template <class T>
class ObjectPool {
public:
    T* Allocate() {
        T* ptr;
        if (!que.empty()) {
            ptr = que.front();
            que.pop();
        }
        else {
            ptr = new T();
        }
        used.insert(ptr);
		return ptr;
    }

    T* TryAllocate() {
        T* ptr = nullptr;
        if (!que.empty()) {
            ptr = que.front();
            que.pop();
            used.insert(ptr);
			return ptr;
        }
        return nullptr;
    }

    void Deallocate(T* object) {
        auto it = used.find(object);
        if (it == used.end()) {
            throw invalid_argument("Error");
        }
        auto ptr = *it;
        que.push(ptr);
        used.erase(it);
    }

    ~ObjectPool() {
        while (!que.empty()) {
            auto ptr = que.front();
            que.pop();
            delete ptr;
        }
        while (!used.empty()) {
            auto ptr = *used.begin();
            used.erase(used.begin());
            delete ptr;
        }
    }

private:
    queue<T*> que;
    set<T*> used;
};

void TestObjectPool() {
    ObjectPool<string> pool;

    auto p1 = pool.Allocate();
    auto p2 = pool.Allocate();
    auto p3 = pool.Allocate();

    *p1 = "first";
    *p2 = "second";
    *p3 = "third";

    pool.Deallocate(p2);
    ASSERT_EQUAL(*pool.Allocate(), "second");

    pool.Deallocate(p3);
    pool.Deallocate(p1);
    ASSERT_EQUAL(*pool.Allocate(), "third");
    ASSERT_EQUAL(*pool.Allocate(), "first");

    pool.Deallocate(p1);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestObjectPool);
    return 0;
}
