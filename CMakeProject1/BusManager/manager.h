#pragma once

#include "json.h"
#include "router.h"

#include <cassert>
#include <memory>
#include <stdexcept>
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
		STOP_INFO,
		ROUTE_INFO
	} Type;

	Response(EResponseType&& type)
		: Type(type)
	{}

	void SetRequestId(int32_t id) {
		Request_id = id;
	}

	int32_t Request_id = -1;
};

class RouteInfoResponse : public Response {
public:
	RouteInfoResponse() : Response(Response::EResponseType::ROUTE_INFO) {}

	RouteInfoResponse(Json::Node&& node)
		: Response(Response::EResponseType::ROUTE_INFO)
		, Info(node)
	{}

	Json::Node Info;
};

class BusInfoResponse: public Response {
public:
	BusInfoResponse() : Response(Response::EResponseType::BUS_INFO) {}

	struct MetricsInfo {
		int CntStops;
		int UniqueStops;
		double PathLength;
		double Curvature;
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
	
	Bus(const vector<string>& path, const unordered_map<string, Stop>& stops, 
		const unordered_map<string, unordered_map<string, double>>& distances_between_stops) {
		RouteLength = 0;
		GeoLength = 0;
		set<string> unique_stops;
		for (size_t i = 0; i < path.size(); ++i) {
			unique_stops.insert(path[i]);
			if (i) {
				double geo_dist = stops.at(path[i - 1]).StopLocation.Distance(stops.at(path[i]).StopLocation);
				GeoLength += geo_dist;
				if (distances_between_stops.count(path[i - 1]) && 
					distances_between_stops.at(path[i - 1]).count(path[i])) {
					RouteLength += distances_between_stops.at(path[i - 1]).at(path[i]);
				}
				else {
					string msg;
					for (const auto& el : distances_between_stops) {
						msg += el.first + ":\n";
						for (const auto& e : el.second) {
							msg += "  " + e.first + ":" + to_string(e.second) + "\n";
						}
					}

					//throw runtime_error("error: " + msg);
					RouteLength += geo_dist;
				}
			}
		}
		CntUnique = static_cast<int>(unique_stops.size());
		Stops = path;
	}
	double RouteLength = 0;
	double GeoLength = 0;
	int CntUnique = 0;
	vector<string> Stops;

	BusInfoResponse GetInfo(const string& name) const {
		double curvature = RouteLength / GeoLength;
		return { name, BusInfoResponse::MetricsInfo{
			static_cast<int>(Stops.size()), CntUnique, RouteLength, curvature } };
	}
};


struct BusManagerSettings {
	BusManagerSettings() 
		: BusWaitTime(0)
		, BusVelocity(0)
	{}

	BusManagerSettings(int bus_wait_time, int bus_velocity)
		: BusWaitTime(bus_wait_time)
		, BusVelocity(bus_velocity)
	{}

	int BusWaitTime;
	int BusVelocity;
};

class BusManager {
public:
	BusManager(const BusManagerSettings& settings)
		: Settings(settings)
	{}

	void AddStop(const string& name, Location location, const unordered_map<string, double>& dist_by_stop) {
		Stops[name] = Stop{ location };
		for (const auto& [stop_name, dist] : dist_by_stop) {
			DistancesBetweenStops[name][stop_name] = dist;
			if (!DistancesBetweenStops.count(stop_name) || !DistancesBetweenStops[stop_name].count(name)) {
				DistancesBetweenStops[stop_name][name] = dist;
			}
		}
	}

	void AddBus(const string& name, const vector<string>& path) {
		Buses[name] = Bus(path, Stops, DistancesBetweenStops);
		for (const auto& stop_name : path) {
			assert(Stops.count(stop_name));
			Stops[stop_name].BusesNames.insert(name);
		}
	}

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

	RouteInfoResponse GetRouteResponse(const string& stop_from, const string& stop_to) {
		using namespace Json;

		size_t from_id = StopIdByName[stop_from];
		size_t to_id = StopIdByName[stop_to];
		auto route = RouteBuilder->BuildRoute(from_id, to_id);
		if (!route) {
			auto node_map = map<string, Node>();
			node_map["error_message"] = Node("not found"s);
			return RouteInfoResponse(Node(node_map));
		}

		auto node_map = map<string, Node>();
		auto result = route.value();
		node_map["total_time"] = Node(result.weight);
		auto node_map_items = vector<Node>();
		vector<int> edges_ids;
		for (size_t i = 0; i < result.edge_count; ++i) {
			auto edge_id = RouteBuilder->GetRouteEdge(result.id, i);
			auto wait_node_map = map<string, Node>();
			wait_node_map["time"] = Node(static_cast<double>(Settings.BusWaitTime));
			wait_node_map["type"] = Node("Wait"s);
			wait_node_map["stop_name"] = Node(Edges[edge_id].StopFrom);
			node_map_items.push_back(Node(wait_node_map));

			auto ride_node_map = map<string, Node>();
			ride_node_map["bus"] = Node(Edges[edge_id].BusName);
			ride_node_map["type"] = Node("Bus"s);
			ride_node_map["time"] = Node(Edges[edge_id].Weight - Settings.BusWaitTime);
			ride_node_map["span_count"] = Node(Edges[edge_id].SpanCount);
			node_map_items.push_back(Node(ride_node_map));
		}
		node_map["items"] = Node(node_map_items);
		return RouteInfoResponse(Node(node_map));
	}
	
	void BuildRoutes() {
		using namespace Graph;

		size_t cur_stop_idx = 0;
		for (const auto& [name, Stop] : Stops) {
			StopIdByName[name] = cur_stop_idx++;
		}

		GraphPtr = make_shared<DirectedWeightedGraph<double>>(Stops.size());

		map<pair<string, string>, tuple<double, string, int>> best_bus_by_2_stops;

		for (const auto& [bus_name, bus] : Buses) {
			for (size_t first_pos = 0; first_pos + 1 < bus.Stops.size(); ++first_pos) {
				double weight = Settings.BusWaitTime;
				for (size_t second_pos = first_pos + 1; second_pos < bus.Stops.size(); ++second_pos) {
					weight += DistancesBetweenStops[bus.Stops[second_pos - 1]][bus.Stops[second_pos]] /
						(Settings.BusVelocity * 1000 / 60.);
					if (!best_bus_by_2_stops.count({ bus.Stops[first_pos], bus.Stops[second_pos] })) {
						best_bus_by_2_stops[{bus.Stops[first_pos], bus.Stops[second_pos]}] =
							make_tuple(weight, bus_name, static_cast<int>(second_pos - first_pos));
					}
					else {
						best_bus_by_2_stops[{bus.Stops[first_pos], bus.Stops[second_pos]}] =
							min(make_tuple(weight, bus_name, static_cast<int>(second_pos - first_pos)),
								best_bus_by_2_stops[{bus.Stops[first_pos], bus.Stops[second_pos]}]);
					}
				}
			}
		}

		for (const auto& el : best_bus_by_2_stops) {
			const auto& [from_stop, to_stop] = el.first;
			const auto& [dist, bus_name, span_count] = el.second;
			Edge<double> edge{ StopIdByName[from_stop], StopIdByName[to_stop], dist };
			auto edge_id = GraphPtr->AddEdge(edge);
			Edges.push_back({ dist, from_stop, to_stop, bus_name, edge_id, span_count });
		}

		RouteBuilder = make_unique<Graph::Router<double>>(*GraphPtr);
	}

private:
	struct EdgeInfo {
		double Weight;
		string StopFrom;
		string StopTo;
		string BusName;
		size_t EdgeId;
		int SpanCount;
	};

	vector<EdgeInfo> Edges;
	unordered_map<string, size_t> StopIdByName;
	unique_ptr<Graph::Router<double>> RouteBuilder;
	shared_ptr<Graph::DirectedWeightedGraph<double>> GraphPtr;

	unordered_map<string, Stop> Stops;
	unordered_map<string, Bus> Buses;
	unordered_map<string, unordered_map<string, double>> DistancesBetweenStops;
	BusManagerSettings Settings;
};
