#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

namespace Utils {

int timeToMinutes(const string& time) {
    if (time == "--" || time.empty()) return 0;
    int h, m;
    char c;
    stringstream ss(time);
    ss >> h >> c >> m;
    return h * 60 + m;
}

string minutesToTime(int minutes) {
    int h = (minutes / 60) % 24;
    int m = minutes % 60;
    ostringstream oss;
    oss << setw(2) << setfill('0') << h << ":"
        << setw(2) << setfill('0') << m;
    return oss.str();
}

void printRoute(const vector<string>& stations) {
    cout << "Route: ";
    for (size_t i = 0; i < stations.size(); ++i) {
        cout << stations[i];
        if (i + 1 < stations.size()) cout << " -> ";
    }
    cout << "\n";
}

}
