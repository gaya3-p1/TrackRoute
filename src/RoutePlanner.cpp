#include "RoutePlanner.h"
#include "Utils.h"
#include <queue>
#include <unordered_map>
#include <climits>
#include <algorithm>
 
using namespace std;
using namespace Utils;
 
RoutePlanner::RoutePlanner(const RailwaySystem& system) : railway(system) {}
 
RouteResult RoutePlanner::findShortestRoute(const string& source, const string& destination) {
    struct State {
        string station;
        int elapsed;
        int arrivalClock; 
        bool operator>(const State& other) const { return elapsed > other.elapsed; }
    };
 
    priority_queue<State, vector<State>, greater<>> pq;
    unordered_map<string, int> dist;
 
    struct EdgeInfo {
        string prevStation;
        int    trainId;
        string trainName;
        string depart;
        string arrive;
        int    travelMinutes;
        int    waitMinutes;
    };
    unordered_map<string, EdgeInfo> best;
 
    for (const auto& t : railway.getTrains())
        for (const auto& s : t.stops)
            dist[s.station] = INT_MAX;
 
    dist[source] = 0;
    pq.push({source, 0, 0});
 
    while (!pq.empty()) {
        auto [station, elapsed, arrivalClock] = pq.top(); pq.pop();
        if (station == destination) break;
        if (elapsed > dist[station]) continue;
 
        for (const auto& train : railway.getTrains()) {
            const auto& stops = train.stops;

            vector<int> cumDep(stops.size(), 0);
            vector<int> cumArr(stops.size(), 0);
            for (size_t i = 1; i < stops.size(); ++i) {
                int dep = timeToMinutes(stops[i-1].departure);
                int arr = timeToMinutes(stops[i].arrival);
                int cost = arr - dep;
                if (cost <= 0) cost += 24 * 60;
                cumArr[i] = cumDep[i-1] + cost;
                if (i + 1 < stops.size()) {
                    int dwell = timeToMinutes(stops[i].departure) - timeToMinutes(stops[i].arrival);
                    if (dwell < 0) dwell += 24 * 60;
                    cumDep[i] = cumArr[i] + dwell;
                }
            }
 
            for (size_t i = 0; i + 1 < stops.size(); ++i) {
                if (stops[i].station == station) {
                    const string& next = stops[i+1].station;
                    int travelCost = cumArr[i+1] - cumDep[i];
                    if (travelCost <= 0) continue;
 
                    int depClock = timeToMinutes(stops[i].departure);
                    int wait = depClock - arrivalClock;
                    if (wait < 0) wait += 24 * 60; 
                    if (station == source) wait = 0;
 
                    int arrClock = timeToMinutes(stops[i+1].arrival);
                    int newElapsed = elapsed + wait + travelCost;
 
                    if (dist.find(next) == dist.end()) dist[next] = INT_MAX;
                    if (newElapsed < dist[next]) {
                        dist[next] = newElapsed;
                        best[next] = {
                            station,
                            train.id,
                            train.name,
                            stops[i].departure,
                            stops[i+1].arrival,
                            travelCost,
                            wait
                        };
                        pq.push({next, newElapsed, arrClock});
                    }
                }
            }
        }
    }
 
    RouteResult result;
    result.changes = 0;
 
    if (dist.find(destination) == dist.end() || dist[destination] == INT_MAX) {
        result.totalMinutes = -1;
        return result;
    }
 
    result.totalMinutes = dist[destination];
 
    vector<RouteSegment> segs;
    string at = destination;
    while (at != source) {
        auto it = best.find(at);
        if (it == best.end()) break;
        const EdgeInfo& e = it->second;
        segs.push_back({e.trainId, e.trainName, e.prevStation, at,
                        e.depart, e.arrive, e.travelMinutes, e.waitMinutes});
        at = e.prevStation;
    }
    reverse(segs.begin(), segs.end());
 
    result.segments = segs;
    result.changes  = (int)segs.size() - 1;
    return result;
}