#include "test_runner.h"

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct Record {
    string id;
    string title;
    string user;
    int timestamp;
    int karma;
};

// Реализуйте этот класс
class Database {
public:
    bool Put(const Record& record);
    const Record* GetById(const string& id) const;
    bool Erase(const string& id);

    template <typename Callback>
    void RangeByTimestamp(int low, int high, Callback callback) const;

    template <typename Callback>
    void RangeByKarma(int low, int high, Callback callback) const;

    template <typename Callback>
    void AllByUser(const string& user, Callback callback) const;

private:
    vector<Record> records;
    unordered_map<string, int> pos_by_id;
    map<int, set<int>> pos_by_timestamp;
    map<int, set<int>> pos_by_karma;
    unordered_map<string, set<int>> pos_by_user;

    template <typename Callback>
    void RangeByString(int low, int high, Callback callback, const map<int, set<int>>& container) const;
};

bool Database::Put(const Record& record) {
    if (pos_by_id.count(record.id)) {
        return false;
    }
    int pos = (int)records.size();
    records.push_back(record);
    pos_by_id[record.id] = pos;
    pos_by_timestamp[record.timestamp].insert(pos);
    pos_by_karma[record.karma].insert(pos);
    pos_by_user[record.user].insert(pos);
    return true;
}

const Record* Database::GetById(const string& id) const {
    auto it = pos_by_id.find(id);
    if (it == pos_by_id.end()) {
        return nullptr;
    }
    return &records[it->second];
}

bool Database::Erase(const string& id) {
    auto it = pos_by_id.find(id);
    if (it == pos_by_id.end()) {
        return false;
    }
    int pos = it->second;
    pos_by_id.erase(records[pos].id);
    pos_by_timestamp[records[pos].timestamp].erase(pos);
    pos_by_karma[records[pos].karma].erase(pos);
    pos_by_user[records[pos].user].erase(pos);
    return true;
}

template<typename Callback>
void Database::RangeByTimestamp(int low, int high, Callback callback) const {
    Database::RangeByString(low, high, callback, pos_by_timestamp);
}

template<typename Callback>
void Database::RangeByKarma(int low, int high, Callback callback) const {
    Database::RangeByString(low, high, callback, pos_by_karma);
}
    
template <typename Callback>
void Database::RangeByString(int low, int high, Callback callback, const map<int, set<int>>& container) const {
    auto it = container.lower_bound(low);
    bool result = true;
    while (it != container.end() && result && it->first <= high) {
        for (auto it2 = it->second.begin(); result && it2 != it->second.end(); ++it2) {
            result = callback(records[*it2]);
        }
        it++;
    }
}

template <typename Callback>
void Database::AllByUser(const string& user, Callback callback) const {
    auto it = pos_by_user.find(user);
    if (it == pos_by_user.end()) {
        return;
    }
    for (auto pos : it->second) {
        bool result = callback(records[pos]);
        if (!result) {
            break;
        }
    }
}

void TestRangeBoundaries() {
    const int good_karma = 1000;
    const int bad_karma = -10;

    Database db;
    db.Put({ "id1", "Hello there", "master", 1536107260, good_karma });
    db.Put({ "id2", "O>>-<", "general2", 1536107260, bad_karma });

    int count = 0;
    db.RangeByKarma(bad_karma, good_karma, [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestSameUser() {
    Database db;
    db.Put({ "id1", "Don't sell", "master", 1536107260, 1000 });
    db.Put({ "id2", "Rethink life", "master", 1536107260, 2000 });

    int count = 0;
    db.AllByUser("master", [&count](const Record&) {
        ++count;
        return true;
        });

    ASSERT_EQUAL(2, count);
}

void TestReplacement() {
    const string final_body = "Feeling sad";

    Database db;
    db.Put({ "id", "Have a hand", "not-master", 1536107260, 10 });
    db.Erase("id");
    db.Put({ "id", final_body, "not-master", 1536107260, -10 });

    auto record = db.GetById("id");
    ASSERT(record != nullptr);
    ASSERT_EQUAL(final_body, record->title);
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestRangeBoundaries);
    RUN_TEST(tr, TestSameUser);
    RUN_TEST(tr, TestReplacement);
    return 0;
}
