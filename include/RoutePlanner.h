#pragma once
#include "RailwaySystem.h"
#include <string>
#include <vector>
 
using namespace std;
 
struct RouteSegment {
    int trainId;
    string trainName;
    string from;
    string to;
    string depart;
    string arrive;
    int travelMinutes;
    int waitMinutes;
};
 
struct RouteResult {
    int totalMinutes;
    int changes;
    vector<RouteSegment> segments;
};
 
class RoutePlanner {
    public:
        RoutePlanner(const RailwaySystem& system);
        RouteResult findShortestRoute(const string& source, const string& destination);
    private:
        const RailwaySystem& railway;
};