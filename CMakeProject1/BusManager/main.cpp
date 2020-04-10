#include "test_runner.h"

#include "manager.h"

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
#include <memory>
#include <ctime>

using namespace std;

void Trim(string& s) {
	while (!s.empty() && (s.back() == ' ' || s.back() == '\n')) {
		s.pop_back();
	}
	reverse(s.begin(), s.end());
	while (!s.empty() && (s.back() == ' ' || s.back() == '\n')) {
		s.pop_back();
	}
	reverse(s.begin(), s.end());
}

vector<string> SplitString(string& s, set<char>&& delims = {}) {
	vector<string> ret;
	string cur = "";
	for (auto ch : s) {
		if (delims.count(ch)) {
			Trim(cur);
			if (!cur.empty()) {
				ret.push_back(cur);
			}
			cur = "";
		}
		else {
			cur += ch;
		}
	}
	Trim(cur);
	if (!cur.empty()) {
		ret.push_back(cur);
	}
	return ret;
}

class Request {
public:
	enum class ERequestType {
		ADD_STOP,
		ADD_BUS,
		QUERY_BUS,
		QUERY_STOP
	} Type;

	Request(ERequestType type)
		: Type(type)
	{}

	virtual void ReadInfo(istream& is) = 0;
};

using RequestHolder = unique_ptr<Request>;

class ModifyRequest : public Request {
public:
    using Request::Request;
	virtual void Process(BusManager& manager) const = 0;
};

class AddStopRequest : public ModifyRequest {
public:
	AddStopRequest() : ModifyRequest(Request::ERequestType::ADD_STOP) {}

	void Process(BusManager& manager) const override {
		manager.AddStop(Name, StopLocation, DistsToStops);
	}

	void ReadInfo(istream& is) override {
		string input;
		getline(is, input);
		auto info = SplitString(input, set<char>{ ':', ',' });
		Name = info[0];
		stringstream ss;
		ss << info[1] << " " << info[2];
		ss >> StopLocation.Latitude >> StopLocation.Longitude;
		for (size_t i = 3; i < info.size(); ++i) {
			auto pos = info[i].find('m');
			assert(pos != string::npos);
			info[i].insert(info[i].begin() + pos, ' ');
			stringstream ss;
			ss << info[i];
			double dist;
			string trash;
			string stop_name;
			ss >> dist >> trash >> trash;
			getline(ss, stop_name);
			stop_name = SplitString(stop_name)[0];
			DistsToStops[stop_name] = dist;
		}
	}

private:
	Location StopLocation;
	string Name;
	unordered_map<string, double> DistsToStops;
};

class AddBusRequest : public ModifyRequest {
public:
	AddBusRequest() : ModifyRequest(Request::ERequestType::ADD_BUS) {}

	void Process(BusManager& manager) const override {
		manager.AddBus(Name, BusStopNames);
	}

	void ReadInfo(istream& is) override {
		string input;
		getline(is, input);
		bool is_cycle = (input.find('>') != string::npos);

		auto info = SplitString(input, set<char>{ '-', '>', ':' });
		Name = info[0];
		copy(info.begin() + 1, info.end(), back_inserter(BusStopNames));
		if (!is_cycle) {
			copy(info.rbegin() + 1, info.rend() - 1, back_inserter(BusStopNames));
		}
	}

private:
	vector<string> BusStopNames;
	string Name;
};

template <typename ResultType>
class ReadRequest : public Request {
public:
    using Request::Request;
	virtual ResultType Process(BusManager& manager) const = 0;
};

class ReadBusInfoRequest : public ReadRequest<BusInfoResponse> {
public:
	ReadBusInfoRequest() : ReadRequest(Request::ERequestType::QUERY_BUS) {}

	BusInfoResponse Process(BusManager& manager) const override {
		return manager.GetBusInfoResponse(BusName);
	}

	void ReadInfo(istream& is) override {
		string input;
		getline(is, input);
		auto info = SplitString(input, set<char>{});
		assert(info.size() == 1);
		BusName = info[0];
	}

private:
	string BusName;
};

class ReadStopInfoRequest : public ReadRequest<StopInfoResponse> {
public:
	ReadStopInfoRequest() : ReadRequest(Request::ERequestType::QUERY_STOP) {}

	StopInfoResponse Process(BusManager& manager) const override {
		return manager.GetStopInfoResponse(StopName);
	}

	void ReadInfo(istream& is) override {
		string input;
		getline(is, input);
		auto info = SplitString(input, set<char>{});
		assert(info.size() == 1);
		StopName = info[0];
	}

private:
	string StopName;
};


const unordered_map<string, Request::ERequestType> ModifyRequestTypeByString = {
	{"Stop", Request::ERequestType::ADD_STOP},
	{"Bus", Request::ERequestType::ADD_BUS}
};

const unordered_map<string, Request::ERequestType> ReadRequestTypeByString = {
	{"Bus", Request::ERequestType::QUERY_BUS},
	{"Stop", Request::ERequestType::QUERY_STOP}
};

RequestHolder CreateRequestHolder(Request::ERequestType type) {
	switch (type) {
		case Request::ERequestType::ADD_BUS:
			return make_unique<AddBusRequest>();
		case Request::ERequestType::ADD_STOP:
			return make_unique<AddStopRequest>();
		case Request::ERequestType::QUERY_BUS:
			return make_unique<ReadBusInfoRequest>();
		case Request::ERequestType::QUERY_STOP:
			return make_unique<ReadStopInfoRequest>();
		default:
			throw "undefined type";
	}
}

void ReadRequests(vector<RequestHolder>& requests, 
	const unordered_map<string, Request::ERequestType>& RequestTypeByString) {
	int queries_count;
	cin >> queries_count;
	for (int i = 0; i < queries_count; ++i) {
		string query_type_string;
		cin >> query_type_string;
		assert(RequestTypeByString.count(query_type_string));
		auto type = RequestTypeByString.at(query_type_string);
		requests.push_back(CreateRequestHolder(type));
		requests.back()->ReadInfo(cin);
	}
}

vector<RequestHolder> ReadAllRequests() {
	vector<RequestHolder> requests;
	ReadRequests(requests, ModifyRequestTypeByString);
	ReadRequests(requests, ReadRequestTypeByString);
	return requests;
}

vector<unique_ptr<Response>> GetResponses(const vector<RequestHolder>& requests) {
	BusManager manager;
	vector<unique_ptr<Response>> responses;
	
	for (auto& request_holder : requests) {
		if (request_holder->Type == Request::ERequestType::ADD_STOP) {
			const auto& request = static_cast<const ModifyRequest&>(*request_holder);
			request.Process(manager);
		}
	}

	for (auto& request_holder : requests) {
		if (request_holder->Type == Request::ERequestType::ADD_BUS) {
			const auto& request = static_cast<const ModifyRequest&>(*request_holder);
			request.Process(manager);
		}
	}

	for (auto& request_holder : requests) {
		if (request_holder->Type == Request::ERequestType::QUERY_BUS) {
			const auto& request = static_cast<const ReadBusInfoRequest&>(*request_holder);
			responses.push_back(make_unique<BusInfoResponse>(request.Process(manager)));
		}
		else if (request_holder->Type == Request::ERequestType::QUERY_STOP) {
			const auto& request = static_cast<const ReadStopInfoRequest&>(*request_holder);
			responses.push_back(make_unique<StopInfoResponse>(request.Process(manager)));
		}
	}
	return responses;
}

void PrintResponses(const vector<unique_ptr<Response>>& responses) {
	for (const auto& response_ptr: responses) {
		if (response_ptr->Type == Response::EResponseType::BUS_INFO) {
			const auto& response = static_cast<const BusInfoResponse&>(*response_ptr);
			cout << "Bus " << response.Name << ": ";
			if (response.Info) {
				cout << response.Info.value().CntStops << " stops on route, " <<
					response.Info.value().UniqueStops << " unique stops, " <<
					fixed << setprecision(6) <<
					response.Info.value().PathLength << " route length, " <<
					response.Info.value().Curvature << " curvature\n";
			}
			else {
				cout << "not found\n";
			}
		}
		else {
			const auto& response = static_cast<const StopInfoResponse&>(*response_ptr);
			cout << "Stop " << response.Name << ": ";
			if (response.Info) {
				if (response.Info.value().Buses.empty()) {
					cout << "no buses\n";
				}
				else {
					cout << "buses";
					for (const auto& bus_name : response.Info.value().Buses) {
						cout << " " << bus_name;
					}
					cout << "\n";
				}
			}
			else {
				cout << "not found\n";
			}
		}
	}
}

int main() {
	FILE* file;
	freopen_s(&file, "C:\\Users\\Admin\\source\\repos\\Alexandr-TS\\CourseraBrownBelt\\CMakeProject1\\BusManager\\a.in", "r", stdin);
	const auto requests = ReadAllRequests();
	const auto responses = GetResponses(requests);
	PrintResponses(responses);
}
