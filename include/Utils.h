#pragma once
#include <string>
#include <vector>

using namespace std;

namespace Utils {
    int timeToMinutes(const string& time);
    string minutesToTime(int minutes);
    void printRoute(const vector<string>& stations);
}
