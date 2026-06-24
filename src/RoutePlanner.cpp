#include "RoutePlanner.h"
#include "Utils.h"
#include <queue>
#include <unordered_map>
#include <iostream>
#include <climits>
#include <algorithm>


using namespace std;
using namespace Utils;

RoutePlanner::RoutePlanner(const RailwaySystem& system) : railway(system) {}

RouteResult RoutePlanner::findShortestRoute(const string& source, const string& destination) {
    // Simplified Dijkstra implementation
    struct State {
        string station;
        int time;
        bool operator>(const State& other) const { return time > other.time; }
    };

    priority_queue<State, vector<State>, greater<>> pq;
    unordered_map<string, int> dist;
    unordered_map<string, string> prev;

    for (const auto& s : railway.getStations()) dist[s] = INT_MAX;
    dist[source] = 0;
    pq.push({source, 0});

    while (!pq.empty()) {
        auto [station, time] = pq.top(); pq.pop();
        if (station == destination) break;

        for (const auto& train : railway.getTrains()) {
            for (size_t i = 0; i + 1 < train.stops.size(); ++i) {
                if (train.stops[i].station == station) {
                    string next = train.stops[i+1].station;
                    int depart = timeToMinutes(train.stops[i].departure);
                    int arrive = timeToMinutes(train.stops[i+1].arrival);
                    int cost = arrive - depart;
                    if (time + cost < dist[next]) {
                        dist[next] = time + cost;
                        prev[next] = station;
                        pq.push({next, dist[next]});
                    }
                }
            }
        }
    }
    RouteResult result;
    result.totalMinutes = dist[destination];
    result.changes = 0;
    vector<string> path;
    for (string at = destination; !at.empty(); at = prev[at]) {
        path.push_back(at);
        if (prev.find(at) == prev.end()) break;
    }
    reverse(path.begin(), path.end());

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        result.segments.push_back({0, "Train", path[i], path[i+1], "", ""});
    }
    return result;
}