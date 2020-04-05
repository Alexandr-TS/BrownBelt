#include "Common.h"
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <memory>
#include <algorithm>
#include <set>

using namespace std;

class LruCache : public ICache {
public:
    LruCache(
        shared_ptr<IBooksUnpacker> books_unpacker,
        const Settings& settings
    )
        : books_unpacker(books_unpacker)
        , settings(settings)
    {
        // реализуйте метод
    }

    BookPtr GetBook(const string& book_name) override {
        {
            shared_lock lock(mutex_);
            auto it = pos_by_name.find(book_name);
            if (it != pos_by_name.end()) {
                int pos = it->second;
                return cache[pos];
            }
        }

        auto book_ptr = books_unpacker->UnpackBook(book_name);

        unique_lock lock(mutex_);
        if (book_ptr->GetContent().size() > settings.max_memory) {
            pos_by_name.clear();
            cache.clear();
            return book_ptr;
        }

        cache.insert(cache.begin(), shared_ptr<IBook>(move(book_ptr)));
        for (auto& el: pos_by_name) {
            el.second++;
        }


        pos_by_name[book_name] = 0;
        total_used_memory += (int)cache[0]->GetContent().size();
        while (total_used_memory > settings.max_memory) {
            total_used_memory -= cache.back()->GetContent().size();
            pos_by_name.erase(cache.back()->GetName());
            cache.pop_back();
        }
        return cache[0];
    }

private:
    vector<BookPtr> cache;
    unordered_map<string, int> pos_by_name;
    int total_used_memory = 0;
    shared_ptr<IBooksUnpacker> books_unpacker;
    Settings settings;
    shared_mutex mutex_;
};


unique_ptr<ICache> MakeCache(
    shared_ptr<IBooksUnpacker> books_unpacker,
    const ICache::Settings& settings
) {
    return make_unique<LruCache>(books_unpacker, settings);
}
