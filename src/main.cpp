#include "DataLoader.h"
#include "RailwaySystem.h"
#include "RoutePlanner.h"
#include "Utils.h"
#include <iostream>
#include <set>
#include <climits>
 
using namespace std;
 
static void printRouteResult(const RouteResult& result) {
    int totalTravel = 0, totalWait = 0;
    for (const auto& seg : result.segments) {
        totalTravel += seg.travelMinutes;
        totalWait   += seg.waitMinutes;
    }
 
    cout << "\n";
    for (size_t i = 0; i < result.segments.size(); ++i) {
        const auto& seg = result.segments[i];
 
        if (seg.waitMinutes > 0) {
            cout << "  [ Wait " << seg.waitMinutes / 60 << "h "
                 << seg.waitMinutes % 60 << "m at " << seg.from << " ]\n\n";
        }
 
        cout << "  Leg " << i + 1 << ": " << seg.from << "  -->  " << seg.to << "\n";
        cout << "  Train  : " << seg.trainName << " (ID: " << seg.trainId << ")\n";
        cout << "  Departs: " << seg.depart << "   Arrives: " << seg.arrive << "\n";
        cout << "  Travel : " << seg.travelMinutes / 60 << "h "
             << seg.travelMinutes % 60 << "m\n";
    }
 
    cout << "\n  ----------------------------------------\n";
    cout << "  Travel time  : " << totalTravel / 60 << "h " << totalTravel % 60 << "m\n";
    cout << "  Waiting time : " << totalWait  / 60 << "h " << totalWait  % 60 << "m\n";
    cout << "  ----------------------------------------\n";
    cout << "  Total time   : " << result.totalMinutes / 60 << "h "
         << result.totalMinutes % 60 << "m"
         << "  (" << result.totalMinutes << " mins)\n";
    cout << "  Train changes: " << result.changes << "\n";
}
 
int main() {
    string filename = "data/trains.csv";
    auto trains = DataLoader::loadCSV(filename);
    RailwaySystem system(trains);
    RoutePlanner planner(system);
 
    cout << "Indian Railway Route Engine\n";
    cout << "Commands:\n";
    cout << "  0. Show commands\n";
    cout << "  1. Search train by ID\n";
    cout << "  2. Search train by name\n";
    cout << "  3. Find least time route\n";
    cout << "  4. List all stations\n";
    cout << "  5. List all trains\n";
    cout << "  6. Exit\n";
 
    while (true) {
        int choice;
        cout << "\nEnter choice: ";
        cin >> choice;
        if (choice == 6) break;
 
        if (choice == 0) {
            cout << "Commands:\n";
            cout << "  0. Show commands\n";
            cout << "  1. Search train by ID\n";
            cout << "  2. Search train by name\n";
            cout << "  3. Find least time route\n";
            cout << "  4. List all stations\n";
            cout << "  5. List all trains\n";
            cout << "  6. Exit\n";
 
        } else if (choice == 1) {
            int id; cout << "Train ID: "; cin >> id;
            const Train* t = system.findTrainById(id);
            if (t) {
                cout << "Found :)\n";
                cout << "NAME : " << t->name << "\n";
                cout << "Timetable:\n";
                system.printTimetable(t->id);
            } else {
                cout << "Not found :(\n";
                cout << "Use option 5 to list all trains\n";
            }
 
        } else if (choice == 2) {
            string name;
            cout << "Train name: ";
            cin.ignore();
            getline(cin, name);
            const Train* t = system.findTrainByName(name);
            if (t) {
                cout << "Found :)\n";
                cout << "Train ID: " << t->id << "\n";
                cout << "Timetable:\n";
                system.printTimetable(t->id);
            } else {
                cout << "Error: Train not found :(\n";
                cout << "Use option 5 to list all trains.\n";
            }
 
        } else if (choice == 3) {
            string src, dst;
            cout << "Source station: "; cin.ignore(); getline(cin, src);
            cout << "Destination station: "; getline(cin, dst);
 
            auto stations = system.getStations();
            set<string> uniqueStations(stations.begin(), stations.end());
            if (!uniqueStations.count(src)) {
                cout << "Error: Source station not found. Use option 4 to list stations.\n";
                continue;
            }
            if (!uniqueStations.count(dst)) {
                cout << "Error: Destination station not found. Use option 4 to list stations.\n";
                continue;
            }
 
            auto result = planner.findShortestRoute(src, dst);
            if (result.totalMinutes == -1) {
                cout << "No route found between " << src << " and " << dst << ".\n";
            } else {
                printRouteResult(result);
            }
 
        } else if (choice == 4) {
            auto stationList = system.getStations();
            set<string> uniqueStations(stationList.begin(), stationList.end());
            cout << "Available Stations:\n";
            for (const auto& s : uniqueStations) cout << "  - " << s << "\n";
 
        } else if (choice == 5) {
            auto allTrains = system.getTrains();
            cout << "Available Trains:\n";
            for (const auto& t : allTrains)
                cout << "  ID: " << t.id << "  Name: " << t.name << "\n";
        }
    }
    return 0;
}