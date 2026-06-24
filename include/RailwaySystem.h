#pragma once
#include "DataLoader.h"
#include <unordered_map>
#include <string>
#include <vector>

using namespace std;

class RailwaySystem {
public:
    RailwaySystem(const vector<Train>& trains);
    const vector<Train>& getTrains() const;
    const Train* findTrainById(int id) const;
    const Train* findTrainByName(const string& name) const;
    void printTimetable(int trainId) const;
    vector<string> getStations() const;
private:
    vector<Train> trains;
    unordered_map<int, Train> trainById;
    unordered_map<string, Train> trainByName;
};
