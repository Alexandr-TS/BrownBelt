#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <cstring>
#include <unordered_map>
#include <utility>
#include <optional>
#include <cmath>
#include <functional>

using namespace std;

const double PI = 3.1415926535;
const double RADIUS = 6371;

struct Location {
	double Latitude = 0.0;
	double Longitude = 0.0;

	double Distance(const Location& other) const {
		pair<double, double> p1 = { Latitude / 180 * PI, Longitude / 180 * PI };
		pair<double, double> p2 = { other.Latitude / 180 * PI, other.Longitude / 180 * PI };
		return acos(sin(p1.first) * sin(p2.first) +
			cos(p1.first) * cos(p2.first) * cos(p1.second - p2.second)) * RADIUS * 1000;
	}

};

class Response {
public:
	enum class EResponseType {
		BUS_INFO,
		STOP_INFO
	} Type;

	Response(EResponseType&& type)
		: Type(type)
	{}
};

class BusInfoResponse: public Response {
public:
	BusInfoResponse() : Response(Response::EResponseType::BUS_INFO) {}

	struct MetricsInfo {
		int CntStops;
		int UniqueStops;
		double PathLength;
	};

	BusInfoResponse(const string& name, optional<MetricsInfo>&& info)
		: Name(name)
		, Info(info)
		, Response(Response::EResponseType::BUS_INFO)
	{}

	string Name;
	optional<MetricsInfo> Info;
};

class StopInfoResponse : public Response {
public:
	StopInfoResponse() : Response(Response::EResponseType::STOP_INFO) {}
	
	struct BusesInfo {
		set<string> Buses;
	};

	StopInfoResponse(const string& name, optional<BusesInfo>&& info)
		: Name(name)
		, Info(info)
		, Response(Response::EResponseType::STOP_INFO)
	{}

	string Name;
	optional<BusesInfo> Info;
};

struct Stop {
	Location StopLocation;
	set<string> BusesNames;
};

struct Bus {
	Bus() {}
	
	Bus(const vector<string>& path, const unordered_map<string, Stop>& stops) {
		Length = 0;
		set<string> unique_stops;
		for (size_t i = 0; i < path.size(); ++i) {
			unique_stops.insert(path[i]);
			if (i) {
				Length += stops.at(path[i - 1]).StopLocation.Distance(stops.at(path[i]).StopLocation);
			}
		}
		CntUnique = static_cast<int>(unique_stops.size());
		Stops = path;
	}
	double Length;
	int CntUnique;
	vector<string> Stops;

	BusInfoResponse GetInfo(const string& name) const {
		return { name, BusInfoResponse::MetricsInfo{static_cast<int>(Stops.size()), CntUnique, Length } };
	}
};

class BusManager {
public:
	BusInfoResponse GetBusInfoResponse(const string& bus_name) {
		auto iter = Buses.find(bus_name);
		if (iter == Buses.end()) {
			return { bus_name, nullopt };
		}
		return iter->second.GetInfo(bus_name);
	}

	StopInfoResponse GetStopInfoResponse(const string& stop_name) {
		auto iter = Stops.find(stop_name);
		if (iter == Stops.end()) {
			return StopInfoResponse{ stop_name, nullopt };
		}
		return StopInfoResponse{ stop_name, StopInfoResponse::BusesInfo{ iter->second.BusesNames } };
	}

	void AddStop(const string& name, Location location) {
		Stops[name] = Stop{ location };
	}

	void AddBus(const string& name, const vector<string>& path) {
		Buses[name] = Bus(path, Stops);
		for (const auto& stop_name : path) {
			Stops[stop_name].BusesNames.insert(name);
		}
	}

private:
	unordered_map<string, Stop> Stops;
	unordered_map<string, Bus> Buses;
};