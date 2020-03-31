#include "test_runner.h"

#include <forward_list>
#include <optional>
#include <algorithm>
#include <utility>
#include <iterator>

using namespace std;

template <typename Type, typename Hasher>
class HashSet {
public:
    using BucketList = forward_list<Type>;

public:
    explicit HashSet(
        size_t num_buckets,
        const Hasher& hasher = {}
    )
        : num_buckets(num_buckets)
        , hasher(hasher) 
    {
        buckets.resize(num_buckets);
    }

    void Add(const Type& value) {
        auto hash = hasher(value) % num_buckets;
        auto cnt = count(buckets[hash].begin(), buckets[hash].end(), value);
        if (cnt == 0) {
            buckets[hash].push_front(value);
        }
    }

    bool Has(const Type& value) const {
        auto hash = hasher(value) % num_buckets;
        auto cnt = count(buckets[hash].begin(), buckets[hash].end(), value);
        return !!cnt;
    }

    void Erase(const Type& value) {
        auto hash = hasher(value) % num_buckets;
        buckets[hash].remove(value);
    }

    const BucketList& GetBucket(const Type& value) const {
        auto hash = hasher(value) % num_buckets;
        return buckets[hash];
    }

private:
    size_t num_buckets;
    vector<forward_list<Type>> buckets;
    Hasher hasher;
};

struct IntHasher {
    size_t operator()(int value) const {
        // Это реальная хеш-функция из libc++, libstdc++.
        // Чтобы она работала хорошо, std::unordered_map
        // использует простые числа для числа бакетов
        return value;
    }
};

struct TestValue {
    int value;

    bool operator==(TestValue other) const {
        return value / 2 == other.value / 2;
    }
};

struct TestValueHasher {
    size_t operator()(TestValue value) const {
        return value.value / 2;
    }
};

void TestSmoke() {
    HashSet<int, IntHasher> hash_buckets(2);
    hash_buckets.Add(3);
    hash_buckets.Add(4);

    ASSERT(hash_buckets.Has(3));
    ASSERT(hash_buckets.Has(4));
    ASSERT(!hash_buckets.Has(5));

    hash_buckets.Erase(3);

    ASSERT(!hash_buckets.Has(3));
    ASSERT(hash_buckets.Has(4));
    ASSERT(!hash_buckets.Has(5));

    hash_buckets.Add(3);
    hash_buckets.Add(5);

    ASSERT(hash_buckets.Has(3));
    ASSERT(hash_buckets.Has(4));
    ASSERT(hash_buckets.Has(5));
}

void TestEmpty() {
    HashSet<int, IntHasher> hash_buckets(10);
    for (int value = 0; value < 10000; ++value) {
        ASSERT(!hash_buckets.Has(value));
    }
}

void TestIdempotency() {
    HashSet<int, IntHasher> hash_buckets(10);
    hash_buckets.Add(5);
    ASSERT(hash_buckets.Has(5));
    hash_buckets.Add(5);
    ASSERT(hash_buckets.Has(5));
    hash_buckets.Erase(5);
    ASSERT(!hash_buckets.Has(5));
    hash_buckets.Erase(5);
    ASSERT(!hash_buckets.Has(5));
}

void TestEquivalence() {
    HashSet<TestValue, TestValueHasher> hash_buckets(10);
    hash_buckets.Add(TestValue{ 2 });
    hash_buckets.Add(TestValue{ 3 });

    ASSERT(hash_buckets.Has(TestValue{ 2 }));
    ASSERT(hash_buckets.Has(TestValue{ 3 }));

    const auto& bucket = hash_buckets.GetBucket(TestValue{ 2 });
    const auto& three_bucket = hash_buckets.GetBucket(TestValue{ 3 });
    ASSERT_EQUAL(&bucket, &three_bucket);

    ASSERT_EQUAL(1, distance(begin(bucket), end(bucket)));
    ASSERT_EQUAL(2, bucket.front().value);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestSmoke);
    RUN_TEST(tr, TestEmpty);
    RUN_TEST(tr, TestIdempotency);
    RUN_TEST(tr, TestEquivalence);
    return 0;
}
