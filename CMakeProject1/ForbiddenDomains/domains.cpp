#include <algorithm>
#include <iostream>
#include <string>
#include <cstring>
#include <string_view>
#include <vector>

using namespace std;

bool IsSubdomain(string subdomain, string domain) {
    if (subdomain.size() < domain.size()) return false;
    return subdomain.substr(0, domain.size()) == domain &&
        (subdomain.size() == domain.size() || subdomain[domain.size()] == '.');
}

vector<string> ReadDomains() {
    size_t count;
    cin >> count;

    vector<string> domains;
    for (size_t i = 0; i < count; ++i) {
        string domain;
        cin >> domain;
        domains.push_back(domain);
    }
    return domains;
}

vector<string> ClearBannedDomains(const vector<string>& banned_domains_) {
    auto banned_domains = banned_domains_;
    for (auto& domain : banned_domains) {
        reverse(begin(domain), end(domain));
    }
    sort(begin(banned_domains), end(banned_domains));
    vector<string> new_banned;

    for (string& domain : banned_domains) {
        if (!new_banned.empty() && IsSubdomain(domain, new_banned.back())) {
            continue;
        }
        else {
            new_banned.push_back(domain);
        }
    }
    return new_banned;
}


int main() {
    vector<string> banned_domains = ClearBannedDomains(ReadDomains());
    vector<string> domains_to_check = ReadDomains();

    for (auto domain : domains_to_check) {
        reverse(domain.begin(), domain.end());
        
        if (const auto it = upper_bound(begin(banned_domains), end(banned_domains), domain);
            it != begin(banned_domains) && IsSubdomain(domain, *prev(it))) {
            cout << "Bad" << endl;
        }
        else {
            cout << "Good" << endl;
        }
    }
    return 0;
}
