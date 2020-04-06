#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <set>
#include <queue>

using namespace std;

class HotelManager {
public:
    void Book(long long time, const string& hotel_name, int client_id, int room_count) {
        clients_count_by_hotel[hotel_name][client_id]++;
        rooms_count_by_hotel[hotel_name] += room_count;
        queries.push({ time, hotel_name, client_id, room_count });
        UpdateQueue(time);
    }

    int GetClientsCount(const string& hotel_name) {
        return static_cast<int>(clients_count_by_hotel[hotel_name].size());
    }

    int GetRoomsCount(const string& hotel_name) {
        return rooms_count_by_hotel[hotel_name];
    }

private:
    struct Booking {
        long long time;
        string hotel_name;
        int client_id;
        int room_count;
    };

    void UpdateQueue(long long time) {
        while (!queries.empty() && queries.front().time + secs_in_day <= time) {
            auto& booking = queries.front();
            auto& it = clients_count_by_hotel[booking.hotel_name][booking.client_id];
            if (!(--it)) {
                clients_count_by_hotel[booking.hotel_name].erase(booking.client_id);
            }
            rooms_count_by_hotel[booking.hotel_name] -= booking.room_count;
            queries.pop();
        }
    }

    queue<Booking> queries;
    unordered_map<string, unordered_map<int, int>> clients_count_by_hotel;
    unordered_map<string, int> rooms_count_by_hotel;

    static const long long secs_in_day = 24 * 60 * 60LL;
};


int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    int cnt_queries;
    cin >> cnt_queries;
    long long time;
    int client_id, room_count;
	string hotel_name;
    HotelManager manager;

    while(cnt_queries--) {
        string query;
        cin >> query;
        if (query == "BOOK") {
            cin >> time >> hotel_name >> client_id >> room_count;
            manager.Book(time, hotel_name, client_id, room_count);
        }
        else if (query == "ROOMS") {
            cin >> hotel_name;
            cout << manager.GetRoomsCount(hotel_name) << "\n";
        }
        else if (query == "CLIENTS") {
            cin >> hotel_name;
            cout << manager.GetClientsCount(hotel_name) << "\n";
        }
    }
    return 0;
}


