#include "test_runner.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <optional>
#include <iomanip>
#include <string>
#include <string_view>
#include <vector>
#include <numeric>
#include <iostream>
#include <cstdio>
#include <ctime>

using namespace std;


class Date {
public:
    Date() 
		: day_(1)
		, month_(1)
		, year_(2000)
    {}

    Date(int day, int month, int year)
        : day_(day)
        , month_(month)
        , year_(year)
    {}

    time_t AsTimestamp() const {
		std::tm t;
		t.tm_sec = 0;
		t.tm_min = 0;
		t.tm_hour = 0;
		t.tm_mday = day_;
		t.tm_mon = month_ - 1;
		t.tm_year = year_ - 1900;
		t.tm_isdst = 0;
		return mktime(&t);
    }

private:
    int day_;
    int month_;
    int year_;
};

istream& operator >> (istream& is, Date& date) {
    int year, month, day;
    string input;
    is >> input;
    stringstream ss;
    ss << input.substr(0, 4) << " " << input.substr(5, 2) << " " << input.substr(8, 2);
    ss >> year >> month >> day;
    assert(2000 <= year && year <= 2099);
    date = Date(day, month, year);
	return is;
}

int ComputeDaysDiff(const Date& date_to, const Date& date_from) {
    const time_t timestamp_to = date_to.AsTimestamp();
    const time_t timestamp_from = date_from.AsTimestamp();
    static const int SECONDS_IN_DAY = 60 * 60 * 24;
    return static_cast<int64_t>(timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}

class BudgetManager {
public:
    explicit BudgetManager(Date first_day = { 1, 1, 2000 })
        : first_day(first_day)
    {}

    long double ComputeIncome(const Date& from, const Date& to) const {
        auto [num_from, num_to] = GetNumsByDateSegment(from, to);
        auto iter_first = money_by_day.lower_bound(num_from);
        auto iter_second = money_by_day.upper_bound(num_to);
        return accumulate(iter_first, iter_second, (long double)0.0,
            [](long double sum, auto el) {
                return sum + el.second.income - el.second.spent;
            }
        );
    }

    void Earn(const Date& from, const Date& to, long double income) {
        auto [num_from, num_to] = GetNumsByDateSegment(from, to);
        auto daily_income = income / (num_to - num_from + 1);
        for (int day = num_from; day <= num_to; ++day) {
            money_by_day[day].income += daily_income;
        }
    }

    void Spend(const Date& from, const Date& to, long double value) {
        auto [num_from, num_to] = GetNumsByDateSegment(from, to);
        auto daily_spend = value / (num_to - num_from + 1);
        for (int day = num_from; day <= num_to; ++day) {
            money_by_day[day].spent += daily_spend;
        }
    }

    void PayTax(const Date& from, const Date& to, int percentage) {
        long double tax_percent = percentage / 100.;
        auto [num_from, num_to] = GetNumsByDateSegment(from, to);
        auto iter_start = money_by_day.lower_bound(num_from);
        auto iter_end = money_by_day.upper_bound(num_to);
        for (auto it = iter_start; it != iter_end; ++it) {
            it->second.income *= (1 - tax_percent);
        }
    }
   
private:
    pair<int, int> GetNumsByDateSegment(const Date& from, const Date& to) const {
        int num_from = ComputeDaysDiff(from, first_day);
        int num_to = ComputeDaysDiff(to, first_day);
        //return { min(num_from, num_to), max(num_from, num_to) };
        assert(num_from <= num_to);
        return { num_from, num_to };
    }

    struct MoneyInfo {
        long double income;
        long double spent;
    };

    map<int, MoneyInfo> money_by_day;
    Date first_day;
};

int main() {
	int queries_count;
	cin >> queries_count;

    BudgetManager budget_manager;

    cout.precision(25);

	while (queries_count--) {
		string type;
        Date date_from, date_to;
        cin >> type >> date_from >> date_to;
        if (type == "Earn") {
            long double income;
            cin >> income;
            budget_manager.Earn(date_from, date_to, income);
        }
        else if (type == "ComputeIncome") {
            auto answer = budget_manager.ComputeIncome(date_from, date_to);
            cout << answer << "\n";
        }
        else if (type == "PayTax") {
            int percentage;
            cin >> percentage;
            budget_manager.PayTax(date_from, date_to, percentage);
        }
        else if (type == "Spend") {
            long double value;
            cin >> value;
            budget_manager.Spend(date_from, date_to, value);
        }
	}
}
