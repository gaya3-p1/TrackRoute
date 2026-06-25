#ifndef RAILWAYSYSTEM_H
#define RAILWAYSYSTEM_H

#include "DataLoader.h"
#include <unordered_map>
#include <string>
#include <vector>


class RailwaySystem {
public:
    RailwaySystem(const std::vector<Train>& trains);
    const std::vector<Train>& getTrains() const;
    const Train* findTrainById(int id) const;
    const Train* findTrainByName(const std::string& name) const;
    void printTimetable(int trainId) const;
    std::vector<std::string> getStations() const;
private:
    std::vector<Train> trains;
    std::unordered_map<int, Train> trainById;
    std::unordered_map<std::string, Train> trainByName;
};

#endif
