#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
 
using namespace std;
 
static void stripCR(string& s) {
    s.erase(remove(s.begin(), s.end(), '\r'), s.end());
}
 
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
 
        stripCR(idStr); stripCR(name); stripCR(station);
        stripCR(arrival); stripCR(departure);
 
        int id = stoi(idStr);
        TrainStop stop{station, arrival, departure};
 
        if (trainMap.find(id) == trainMap.end())
            trainMap[id] = Train{id, name, {}};
        trainMap[id].stops.push_back(stop);
    }
 
    for (auto& kv : trainMap) {
    trains.push_back(kv.second);
    }

    return trains;
}