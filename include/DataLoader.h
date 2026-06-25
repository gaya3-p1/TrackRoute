#ifndef DATALOADER_H
#define DATALOADER_H

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

struct TrainStop {
    std::string station;
    std::string arrival;
    std::string departure;
};

struct Train {
    int id;
    std::string name;
    std::vector<TrainStop> stops;
};

class DataLoader {
public:
    static std::vector<Train> loadCSV(const std::string& filename);
};

#endif