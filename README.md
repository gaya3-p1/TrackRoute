# Indian Railway Route Optimizer (TrackRoute)
An easy way to find the earliest possible travel from two stations that don't have a direct railway option.

There is a CSV file containing 400+ virtual trains passing through 25 stations.

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

## How to Run
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
