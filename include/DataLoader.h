#pragma once
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct TrainStop {
    string station;
    string arrival;
    string departure;
};

struct Train {
    int id;
    string name;
    vector<TrainStop> stops;
};

class DataLoader {
public:
    static vector<Train> loadCSV(const string& filename);
};