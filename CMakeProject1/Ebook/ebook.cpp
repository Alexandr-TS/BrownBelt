#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <array>

using namespace std;

class ReadingManager {
public:
    ReadingManager() {
        fill(pages_by_user.begin(), pages_by_user.end(), 0);
        fill(pref_sum.begin(), pref_sum.end(), 0);
    }

    void Read(int user_id, int page_count) {
        int prev_cnt = pages_by_user[user_id];
        for (int i = prev_cnt + 1; i <= page_count; ++i) {
            ++pref_sum[i];
        }
        pages_by_user[user_id] = page_count;
    }

    double Cheer(int user_id) const {
        if (pages_by_user[user_id] == 0) {
            return 0;
        }
        int total_users = pref_sum[1];
        if (total_users == 1) {
            return 1;
        }
        int users_same = pref_sum[pages_by_user[user_id]];
        return (double)(total_users - users_same) / (total_users - 1);
    }

private:
    static const int MAX_USERS = 100'001;
    static const int MAX_PAGES = 1000;
    array<int, MAX_USERS> pages_by_user;
    array<int, MAX_PAGES + 1> pref_sum;
};


int main() {
    // Для ускорения чтения данных отключается синхронизация
    // cin и cout с stdio,
    // а также выполняется отвязка cin от cout
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ReadingManager manager;

    int query_count;
    cin >> query_count;

    for (int query_id = 0; query_id < query_count; ++query_id) {
        string query_type;
        cin >> query_type;
        int user_id;
        cin >> user_id;

        if (query_type == "READ") {
            int page_count;
            cin >> page_count;
            manager.Read(user_id, page_count);
        }
        else if (query_type == "CHEER") {
            cout << setprecision(6) << manager.Cheer(user_id) << "\n";
        }
    }

    return 0;
}