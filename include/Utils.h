#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>


namespace Utils {
    int timeToMinutes(const atd::string& time);
    std::string minutesToTime(int minutes);
    void printRoute(const std::vector<std::string>& stations);
}

#endif 
