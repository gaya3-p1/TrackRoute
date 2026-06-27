# Indian Railway Route Optimizer (TrackRoute)
 
A C++ console application that models an Indian railway network, lets you look up trains and timetables, and finds the fastest route between any two stations — including multi-train journeys with optimal connections.
 
---
 
## Features
 
- Load train schedules from a CSV file
- Search trains by ID or name
- View full timetables for any train
- List all stations and trains in the network
- Find the least-time route between two stations (handles transfers and waiting time)
---
 
## Project Structure
 
```
 trains.csv          # Train schedule data
 DataLoader.h / .cpp     # CSV parsing into Train/TrainStop structs
 RailwaySystem.h / .cpp  # In-memory store; lookup and timetable display
 RoutePlanner.h / .cpp   # Dijkstra-based shortest-time route finder
 Utils.h / .cpp          # Time conversion helpers
 main.cpp                # CLI entry point
```
 
---  

## Available Stations:  

| | | | |
| --- | --- | --- | --- |
| AHMEDABAD | HYDERABAD | MADURAI | SURAT |
| AMRITSAR | INDORE | MUMBAI | VARANASI |
| BANGALORE | JAIPUR | NAGPUR | VISAKHAPATNAM |
| BHOPAL | KOCHI | PATNA | GUWAHATI |
| BHUBANESWAR | KOLKATA | PUNE | |
| CHANDIGARH | LUCKNOW | RANCHI | |
| CHENNAI | COIMBATORE | DELHI | |

---  


## CSV Format
 
```
train_id,train_name,station,arrival,departure
12001,Deccan Express 1,DELHI,--,06:00
12001,Deccan Express 1,BHOPAL,13:00,13:15
...
```
 
- Use `--` for arrival at the origin or departure at the terminus.
- Times are in `HH:MM` (24-hour) format.
- Overnight trains (crossing midnight) are handled automatically.
---
 
## How the Route Finder Works
 
The planner uses **Dijkstra's algorithm** where the cost is total elapsed time (travel + waiting for a connecting train). At each station it explores every train that stops there and computes:
 
```
new_elapsed = current_elapsed + wait_for_train + travel_time_to_next_stop
```
 
Waiting time at the source is always 0. For connections, the wait is the gap between your arrival and the next train's departure (wrapping midnight if needed). The path is reconstructed by backtracking through a `best[]` map from destination to source.
 
---

## CLI

To use the TrackRoute Optimizer, run the following commands in your terminal:  

```bash
make
./route_planner

Indian Railway Route Engine
Commands:
  0. Show commands
  1. Search train by ID
  2. Search train by name
  3. Find least time route
  4. List all stations
  5. List all trains
  6. Exit
 ```
Finding the Least Time Route :

```bash
Source station: HYDERABAD
Destination station: SURAT

  Leg 1: HYDERABAD  -->  PUNE
  Train  : Konkan Kanya Ret 8 (ID: 102793)
  Departs: 20:15   Arrives: 04:45
  Travel : 8h 30m
  [ Wait 0h 5m at PUNE ]

  Leg 2: PUNE  -->  MUMBAI
  Train  : East Coast Express Ret 5 (ID: 102848)
  Departs: 04:50   Arrives: 08:20
  Travel : 3h 30m
  [ Wait 1h 40m at MUMBAI ]

  Leg 3: MUMBAI  -->  SURAT
  Train  : Saurashtra Express Ret 6 (ID: 109015)
  Departs: 10:00   Arrives: 12:30
  Travel : 2h 30m ```

  ----------------------------------------
  Travel time  : 14h 30m
  Waiting time : 1h 45m
  ----------------------------------------

  Total time   : 16h 15m  (975 mins)
  Train changes: 2
```
---  
## Note
 
- Station names are case-sensitive; use the exact names shown by option 4.
- The engine models a single daily timetable (no multi-day scheduling).
- Train order in CSV does not matter; stops are stored in file order per train.
