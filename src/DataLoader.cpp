#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

vector<Train> DataLoader::loadCSV(const string& filename) {
    vector<Train> trains;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << "\n";
        return trains;
    }

    string line;
    unordered_map<int, Train> trainMap;

    getline(file, line);
    while (getline(file, line)) {
        stringstream ss(line);
        string idStr, name, station, arrival, departure;
        getline(ss, idStr, ',');
        getline(ss, name, ',');
        getline(ss, station, ',');
        getline(ss, arrival, ',');
        getline(ss, departure, ',');

        int id = std::stoi(idStr);
        TrainStop stop{station, arrival, departure};

        if (trainMap.find(id) == trainMap.end()) {
            trainMap[id] = Train{id, name, {}};
        }
        trainMap[id].stops.push_back(stop);
    }

    for (auto& [id, train] : trainMap) {
        trains.push_back(train);
    }
    return trains;
}