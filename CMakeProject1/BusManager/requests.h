#pragma once
#include "manager.h"
#include "json.h"

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
using namespace Json;

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

	virtual void ReadInfo(istream&) = 0;
	virtual void ReadInfo(const Node&) = 0;
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
		auto info = StringUtils::SplitString(input, set<char>{ ':', ',' });
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
			stop_name = StringUtils::SplitString(stop_name)[0];
			DistsToStops[stop_name] = dist;
		}
	}

	void ReadInfo(const Node& node) override {
		const auto& node_map = node.AsMap();
		StopLocation = { node_map.at("latitude").AsDouble(), node_map.at("longitude").AsDouble() };
		Name = node_map.at("name").AsString();
		if (node_map.count("road_distances")) {
			for (const auto& [stop_name, dist] : node_map.at("road_distances").AsMap()) {
				DistsToStops[stop_name] = dist.AsDouble();
			}
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

		auto info = StringUtils::SplitString(input, set<char>{ '-', '>', ':' });
		Name = info[0];
		copy(info.begin() + 1, info.end(), back_inserter(BusStopNames));
		if (!is_cycle) {
			copy(info.rbegin() + 1, info.rend() - 1, back_inserter(BusStopNames));
		}
	}

	void ReadInfo(const Node& node) override {
		const auto& node_map = node.AsMap();
		Name = node_map.at("name").AsString();
		const auto& stop_nodes = node_map.at("stops").AsArray();
		for (const auto& stop_node : stop_nodes) {
			BusStopNames.push_back(stop_node.AsString());
		}
		if (node_map.at("is_roundtrip").AsDouble() < 0.5) { // false
			vector<string> reversed_path{ BusStopNames.begin(), prev(BusStopNames.end()) };
			copy(reversed_path.begin(), reversed_path.end(), back_inserter(BusStopNames));
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

protected:
	int32_t Request_id = -1;
};

class ReadBusInfoRequest : public ReadRequest<BusInfoResponse> {
public:
	ReadBusInfoRequest() : ReadRequest(Request::ERequestType::QUERY_BUS) {}

	BusInfoResponse Process(BusManager& manager) const override {
		auto response = manager.GetBusInfoResponse(BusName);
		response.SetRequestId(Request_id);
		return response;
	}

	void ReadInfo(istream& is) override {
		string input;
		getline(is, input);
		auto info = StringUtils::SplitString(input);
		assert(info.size() == 1);
		BusName = info[0];
	}

	void ReadInfo(const Node& node) override {
		BusName = node.AsMap().at("name").AsString();
		Request_id = static_cast<int>(node.AsMap().at("id").AsDouble());
	}

private:
	string BusName;
};

class ReadStopInfoRequest : public ReadRequest<StopInfoResponse> {
public:
	ReadStopInfoRequest() : ReadRequest(Request::ERequestType::QUERY_STOP) {}

	StopInfoResponse Process(BusManager& manager) const override {
		auto response = manager.GetStopInfoResponse(StopName);
		response.SetRequestId(Request_id);
		return response;
	}

	void ReadInfo(istream& is) override {
		string input;
		getline(is, input);
		auto info = StringUtils::SplitString(input);
		assert(info.size() == 1);
		StopName = info[0];
	}

	void ReadInfo(const Node& node) override {
		StopName = node.AsMap().at("name").AsString();
		Request_id = static_cast<int>(node.AsMap().at("id").AsDouble());
	}

private:
	string StopName;
};
