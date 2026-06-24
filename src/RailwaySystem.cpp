#include "RailwaySystem.h"
#include <iostream>

using namespace std;
RailwaySystem::RailwaySystem(const vector<Train>& trains) : trains(trains) {
    for (const auto& t : trains) {
        trainById[t.id] = t;
        trainByName[t.name] = t;
    }
}

const vector<Train>& RailwaySystem::getTrains() const { return trains; }

const Train* RailwaySystem::findTrainById(int id) const {
    auto it = trainById.find(id);
    return it != trainById.end() ? &it->second : nullptr;
}

const Train* RailwaySystem::findTrainByName(const string& name) const {
    auto it = trainByName.find(name);
    return it != trainByName.end() ? &it->second : nullptr;
}

void RailwaySystem::printTimetable(int trainId) const {
    const Train* train = findTrainById(trainId);
    if (!train) {
        cout << "Train not found.\n";
        return;
    }

    cout << "Timetable for " << train->name << " (ID: " << train->id << "):\n";
    for (const auto& stop : train->stops) {
        cout << " - " << stop.station
                  << " | Arr: " << (stop.arrival.empty() ? "--" : stop.arrival)
                  << " | Dep: " << (stop.departure.empty() ? "--" : stop.departure)
                  << "\n";
    }
}

vector<string> RailwaySystem::getStations() const {
    vector<string> stations;
    for (const auto& t : trains) {
        for (const auto& s : t.stops) {
            stations.push_back(s.station);
        }
    }
    return stations;
}
