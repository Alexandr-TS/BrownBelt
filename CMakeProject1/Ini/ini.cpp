#include "ini.h"
#include <cassert>

namespace Ini {

//using Section = unordered_map<string, string>;
    //unordered_map<string, Section> sections;

Section& Document::AddSection(string name) {
    if (!sections.count(name)) {
        sections[name] = {};
    }
    return sections[name];
}

const Section& Document::GetSection(const string& name) const {
    return sections.at(name);
}

size_t Document::SectionCount() const {
    return sections.size();
}

Document Load(istream& input) {
    string section_name = "";
    Document doc;
    Section* sec_ptr = nullptr;

    string row;
    while (getline(input, row)) {
		while (!row.empty() && (row.back() == ' ' || row.back() == '\n' || row.back() == '\t')) {
			row.pop_back();
		}
        if (row.empty()) {
            continue;
        }
        if (row[0] == '[' && row.back() == ']') {
            section_name = row;
            section_name.pop_back();
            section_name.erase(section_name.begin());
            sec_ptr = &doc.AddSection(section_name);
        }
        else {
            auto pos = row.find('=');
            assert(pos != string::npos);
            assert(sec_ptr != nullptr);
            sec_ptr->insert({ row.substr(0, pos), row.substr(pos + 1, (int)row.size() - pos - 1) });
        }
    }
    return doc;
}

}