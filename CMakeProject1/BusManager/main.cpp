#include "test_runner.h"

#include "manager.h"
#include "utils.h"
#include "requests.h"

using namespace std;

const unordered_map<string, Request::ERequestType> ModifyRequestTypeByString = {
	{"Stop", Request::ERequestType::ADD_STOP},
	{"Bus", Request::ERequestType::ADD_BUS}
};

const unordered_map<string, Request::ERequestType> ReadRequestTypeByString = {
	{"Bus", Request::ERequestType::QUERY_BUS},
	{"Stop", Request::ERequestType::QUERY_STOP},
	{"Route", Request::ERequestType::QUERY_ROUTE}
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

void ReadRequestsCin(vector<RequestHolder>& requests, 
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

vector<RequestHolder> ReadAllRequestsCin() {
	vector<RequestHolder> requests;
	ReadRequestsCin(requests, ModifyRequestTypeByString);
	ReadRequestsCin(requests, ReadRequestTypeByString);
	return requests;
}

void ReadRequestsJson(vector<RequestHolder>& requests, const Node& node,
	const unordered_map<string, Request::ERequestType>& RequestTypeByString) {
	for (const auto& query_node : node.AsArray()) {
		auto type = RequestTypeByString.at(query_node.AsMap().at("type").AsString());
		requests.push_back(CreateRequestHolder(type));
		requests.back()->ReadInfo(query_node);
	}
}


pair<BusManagerSettings, vector<RequestHolder>> ReadAllRequestsJson() {
	auto document = Load(cin);
	vector<RequestHolder> requests;

	assert((int)document.GetRoot().AsMap().size() == 2);

	const auto& modify_requests = document.GetRoot().AsMap().at("base_requests");
	ReadRequestsJson(requests, modify_requests, ModifyRequestTypeByString);

	const auto& read_requests = document.GetRoot().AsMap().at("stat_requests");
	ReadRequestsJson(requests, read_requests, ReadRequestTypeByString);

	const auto& settings_info = document.GetRoot().AsMap().at("routing_settings").AsMap();
	auto settings = BusManagerSettings(
		static_cast<int>(settings_info.at("bus_wait_time").AsDouble()),
		static_cast<int>(settings_info.at("bus_velocity").AsDouble())
	);

	return { settings, move(requests) };
}

vector<unique_ptr<Response>> GetResponses(
	const BusManagerSettings& settings, const vector<RequestHolder>& requests) {
	BusManager manager(settings);
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

	manager.BuildRoutes();

	for (auto& request_holder : requests) {
		if (request_holder->Type == Request::ERequestType::QUERY_BUS) {
			const auto& request = static_cast<const ReadBusInfoRequest&>(*request_holder);
			responses.push_back(make_unique<BusInfoResponse>(request.Process(manager)));
		}
		else if (request_holder->Type == Request::ERequestType::QUERY_STOP) {
			const auto& request = static_cast<const ReadStopInfoRequest&>(*request_holder);
			responses.push_back(make_unique<StopInfoResponse>(request.Process(manager)));
		}
		else if (request_holder->Type == Request::ERequestType::QUERY_ROUTE) {
			const auto& request = static_cast<const ReadRouteInfoRequest&>(*request_holder);
			responses.push_back(make_unique<RouteInfoResponse>(request.Process(manager)));
		}
	}
	return responses;
}

void PrintResponsesCout(const vector<unique_ptr<Response>>& responses) {
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
		else if (response_ptr->Type == Response::EResponseType::STOP_INFO) {
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
		else if (response_ptr->Type == Response::EResponseType::ROUTE_INFO) {
			throw runtime_error("Not implemented Response to print");
		}
		else {
			throw runtime_error("Not implemented Response to print");
		}
	}
}

void PrintResponsesJson(const vector<unique_ptr<Response>>& responses) {
	auto result_vec = vector<Node>();

	for (const auto& response_ptr: responses) {
		Node response_node;
		if (response_ptr->Type == Response::EResponseType::BUS_INFO) {
			auto cur_node = map<string, Node>{};
			const auto& response = static_cast<const BusInfoResponse&>(*response_ptr);
			cur_node["request_id"] = Node(static_cast<double>(response.Request_id));
			if (response.Info) {
				cur_node["stop_count"] = Node(static_cast<double>(response.Info.value().CntStops));
				cur_node["unique_stop_count"] = Node(static_cast<double>(response.Info.value().UniqueStops));
				cur_node["curvature"] = Node(response.Info.value().Curvature);
				cur_node["route_length"] = Node(response.Info.value().PathLength);
			}
			else {
				cur_node["error_message"] = Node("not found"s);
			}
			response_node = Node(cur_node);
		}
		else if (response_ptr->Type == Response::EResponseType::STOP_INFO) {
			auto cur_node = map<string, Node>{};
			const auto& response = static_cast<const StopInfoResponse&>(*response_ptr);
			cur_node["request_id"] = Node(static_cast<double>(response.Request_id));
			if (response.Info) {
				auto buses_vector = vector<Node>();
				for (const auto& bus_name : response.Info.value().Buses) {
					buses_vector.push_back(Node(bus_name));
				}
				cur_node["buses"] = buses_vector;
			}
			else {
				cur_node["error_message"] = Node("not found"s);
			}
			response_node = Node(cur_node);
		}
		else if (response_ptr->Type == Response::EResponseType::ROUTE_INFO) {
			const auto& response = static_cast<const RouteInfoResponse&>(*response_ptr);
			response_node = response.Info;
		}
		else {
			throw runtime_error("Not implemented Response to print");
		}
		result_vec.push_back(response_node);
	}

	auto result_node = Node(result_vec);
	result_node.Print(cout);
}

int main() {
	FILE* file;
	freopen_s(&file, "C:\\Users\\Admin\\source\\repos\\Alexandr-TS\\CourseraBrownBelt\\CMakeProject1\\BusManager\\a.in", "r", stdin);
	auto requests = ReadAllRequestsJson();
	const auto responses = GetResponses(requests.first, move(requests.second));
	PrintResponsesJson(responses);
}
