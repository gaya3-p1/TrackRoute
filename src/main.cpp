#include "DataLoader.h"
#include "RailwaySystem.h"
#include "RoutePlanner.h"
#include "Utils.h"
#include <iostream>
#include <set>

using namespace std;

int main() {
    string filename = "data/trains.csv";
    auto trains = DataLoader::loadCSV(filename);
    RailwaySystem system(trains);
    RoutePlanner planner(system);

    cout << "Indian Railway Route Engine" << endl;
    cout << "Commands:" << endl;
    cout << "0. Show commands" << endl;
    cout << "1. Search train by ID" << endl;
    cout << "2. Search train by name" << endl;
    cout << "3. Find least time route" << endl;
    cout << "4. List all stations" << endl;
    cout << "5. List all trains" << endl;
    cout << "6. Exit" << endl;

    while (true) {
        int choice;
        cout << "\nEnter choice: ";
        cin >> choice;
        if (choice == 6) break;

        if (choice == 0) {
            cout << "Commands:" << endl;
            cout << "0. Show commands" << endl;
            cout << "1. Search train by ID" << endl;
            cout << "2. Search train by name" << endl;
            cout << "3. Find least time route" << endl;
            cout << "4. List all stations" << endl;
            cout << "5. List all trains" << endl;
            cout << "6. Exit" << endl;
        } else if (choice == 1) {
            int id; cout << "Train ID: "; cin >> id;
            const Train* t = system.findTrainById(id);
            if (t){ 
                cout << "Found :)" << endl;
                cout<<"NAME : " <<t->name<<endl;
                cout<<"Timetable: "<<endl;
                system.printTimetable(t->id);

            } else {
                cout << "Not found :(" << endl;
                cout <<"Use option 5 to list all trains"<<endl;
            }
        } else if (choice == 2) {
            string name; 
            cout << "Train name: "; 
            cin.ignore(); 
            getline(std::cin, name);

            const Train* t = system.findTrainByName(name);
            if (t) {
                cout << "Found :)"<<endl;
                cout << "Train ID: " << t->id << endl;
                cout << "Timetable:" << endl;
                system.printTimetable(t->id);
            } else {
                cout << "Error: Train not found :(" << endl;
                cout << "Use option 5 to list all trains." << endl;
            }
        } else if (choice == 3) {
            string src, dst;
            cout << "Source station: "; cin.ignore(); getline(cin, src);
            cout << "Destination station: "; getline(cin, dst);

            // Validate stations
            auto stations = system.getStations();
            set<string> uniqueStations(stations.begin(), stations.end());
            if (uniqueStations.find(src) == uniqueStations.end()) {
                cout << "Error: Source station not found. Use option 4 to list stations."<<endl;
                continue;
            }
            if (uniqueStations.find(dst) == uniqueStations.end()) {
                cout << "Error: Destination station not found. Use option 4 to list stations."<<endl;
                continue;
            }

            auto result = planner.findShortestRoute(src, dst);
            cout << "Total minutes: " << result.totalMinutes << endl;
            cout << "Segments: " << result.segments.size() << endl;
            Utils::printRoute({src, dst}); // simplified
        } else if (choice == 4) {
            auto stations = system.getStations();
            set<string> uniqueStations(stations.begin(), stations.end());
            cout << "Available Stations:\n";
            for (const auto& s : uniqueStations) cout << " - " << s << endl;
        } else if (choice == 5) {
            auto trains = system.getTrains();
            cout << "Available Trains:" << endl;
            for (const auto& t : trains) {
                cout << "ID: " << t.id << "  Name: " << t.name << endl;
            }
        }
    }
    return 0;
}

