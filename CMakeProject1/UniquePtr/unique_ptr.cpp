#include "test_runner.h"

#include <cstddef>  // нужно для nullptr_t

using namespace std;

// Реализуйте шаблон класса UniquePtr
template <typename T>
class UniquePtr {
private:
    T* obj;

public:
    UniquePtr()
        : obj(nullptr)
    {}

    UniquePtr(T* ptr)
        : obj(ptr)
    {}

    UniquePtr(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) {
        obj = other.obj;
        other.obj = nullptr;
    }

    UniquePtr& operator = (const UniquePtr&) = delete;

    UniquePtr& operator = (nullptr_t) {
        Reset(nullptr);
        return *this;
    }

    UniquePtr& operator = (UniquePtr&& other) {
        if (this == &other) return *this;
        Reset(other.obj);
        other.obj = nullptr;
        return *this;
    }

    ~UniquePtr() {
        delete obj;
    }

    T& operator * () const {
        return *obj;
    }

    T* operator -> () const {
        return obj;
    }

    T* Release() {
        auto ptr = obj;
        obj = nullptr;
        return ptr;
    }

    void Reset(T* ptr) {
        delete obj;
        obj = ptr;
    }

    void Swap(UniquePtr& other) {
        swap(other.obj, obj);
    }

    T* Get() const {
        return obj;
    }
};


struct Item {
    static int counter;
    int value;
    Item(int v = 0) : value(v) {
        ++counter;
    }
    Item(const Item& other) : value(other.value) {
        ++counter;
    }
    ~Item() {
        --counter;
    }
};

int Item::counter = 0;


void TestLifetime() {
    Item::counter = 0;
    {
        UniquePtr<Item> ptr(new Item);
        ASSERT_EQUAL(Item::counter, 1);

        ptr.Reset(new Item);
        ASSERT_EQUAL(Item::counter, 1);
    }
    ASSERT_EQUAL(Item::counter, 0);

    {
        UniquePtr<Item> ptr(new Item);
        ASSERT_EQUAL(Item::counter, 1);

        auto rawPtr = ptr.Release();
        ASSERT_EQUAL(Item::counter, 1);

        delete rawPtr;
        ASSERT_EQUAL(Item::counter, 0);
    }
    ASSERT_EQUAL(Item::counter, 0);
}

void TestGetters() {
    UniquePtr<Item> ptr(new Item(42));
    ASSERT_EQUAL(ptr.Get()->value, 42);
    ASSERT_EQUAL((*ptr).value, 42);
    ASSERT_EQUAL(ptr->value, 42);

}

void TestMy() {
    UniquePtr<int> ptr1(new int(1));
    UniquePtr<int> ptr2(move(ptr1));
    UniquePtr<int> ptr3;
    ptr3 = nullptr;
    ptr3 = move(ptr2);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestLifetime);
    RUN_TEST(tr, TestGetters);
    RUN_TEST(tr, TestMy);
}
