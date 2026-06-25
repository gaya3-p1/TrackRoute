#ifndef ROUTEPLANNER_H
#define ROUTEPLANNER_H

#include "RailwaySystem.h"
#include <string>
#include <vector>
 
 
struct RouteSegment {
    int trainId;
    std::string trainName;
    std::string from;
    std::string to;
    std::string depart;
    std::string arrive;
    int travelMinutes;
    int waitMinutes;
};
 
struct RouteResult {
    int totalMinutes;
    int changes;
    std::vector<RouteSegment> segments;
};
 
class RoutePlanner {
    public:
        RoutePlanner(const RailwaySystem& system);
        RouteResult findShortestRoute(const std::string& source, const string& destination);
    private:
        const RailwaySystem& railway;
};

#endif