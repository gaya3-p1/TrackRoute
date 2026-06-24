#include <bits/stdc++.h>
using namespace std;

// ─── Time Utilities ───────────────────────────────────────────────────────────

// Parse "HH:MM" -> minutes from midnight. Returns -1 for "--" or invalid.
int parseTime(const string& s) {
    if (s.size() < 5 || s[2] != ':') return -1;
    int h = stoi(s.substr(0, 2));
    int m = stoi(s.substr(3, 2));
    if (h < 0 || h > 23 || m < 0 || m > 59) return -1;
    return h * 60 + m;
}

// Format absolute minutes (may exceed 1440 for multi-day journeys) as HH:MM.
// Appends "(Day+N)" when the time overflows into a later day.
string fmtTime(int totalMins) {
    int day = totalMins / 1440;
    int t   = totalMins % 1440;
    int h = t / 60, m = t % 60;
    ostringstream oss;
    oss << setfill('0') << setw(2) << h << ":" << setw(2) << m;
    if (day > 0) oss << "  (Day+" << day << ")";
    return oss.str();
}

// Format a duration in minutes as "Xd Yh ZZm".
string fmtDur(int mins) {
    if (mins < 0) mins = 0;
    int d = mins / 1440; mins %= 1440;
    int h = mins / 60,   m = mins % 60;
    ostringstream oss;
    if (d > 0) oss << d << "d ";
    oss << h << "h " << setfill('0') << setw(2) << m << "m";
    return oss.str();
}

// ─── Core Data Structures ─────────────────────────────────────────────────────

struct Stop {
    string trainId;
    string trainName;
    string station;
    int cumArr;   // minutes from train's first departure (-1 for the origin stop)
    int cumDep;   // minutes from train's first departure (-1 for the terminus stop)
};

// trainId -> ordered stops (origin first, terminus last)
map<string, vector<Stop>> schedules;

// station name -> list of trainIds that stop there
map<string, vector<string>> stationIndex;

// all station names (populated on load)
set<string> allStations;

// ─── CSV Loading ──────────────────────────────────────────────────────────────

static void trimStr(string& s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '"' || s.front() == '\r'))
        s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '"' || s.back() == '\r'))
        s.pop_back();
}

// Load train schedules from a CSV file.
// Handles overnight trains: if a stop's raw time regresses, the day counter
// increments automatically so cumulative times stay monotonically increasing.
void loadCSV(const string& fname) {
    ifstream f(fname);
    if (!f.is_open()) {
        cerr << "\n  ERROR: Cannot open \"" << fname << "\"\n";
        cerr << "  Run:  make data   to generate the sample dataset first.\n\n";
        exit(1);
    }

    string line;
    getline(f, line); // skip header

    // Per-train day tracking for overnight detection
    map<string, int> trainPrevCum;

    while (getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (!line.empty() && line.back() == '\r') line.pop_back();

        istringstream ss(line);
        string id, name, station, arrS, depS;
        getline(ss, id,      ',');
        getline(ss, name,    ',');
        getline(ss, station, ',');
        getline(ss, arrS,    ',');
        getline(ss, depS);

        trimStr(id); trimStr(name); trimStr(station);
        trimStr(arrS); trimStr(depS);
        if (id.empty() || station.empty()) continue;

        int arrRaw = parseTime(arrS);
        int depRaw = parseTime(depS);

        auto& stops = schedules[id];
        int cumArr = -1, cumDep = -1;

        if (stops.empty()) {
            // Origin stop: cumDep is the raw departure time (day 0 reference)
            cumArr = -1;
            cumDep = depRaw;
            if (cumDep >= 0) trainPrevCum[id] = cumDep;
        } else {
            int prevCum = trainPrevCum.count(id) ? trainPrevCum[id] : 0;

            // Advance arrival past midnight if needed
            if (arrRaw >= 0) {
                int dayBase = (prevCum / 1440) * 1440;
                cumArr = dayBase + arrRaw;
                if (cumArr < prevCum) cumArr += 1440;
                trainPrevCum[id] = cumArr;
                prevCum = cumArr;
            }

            // Advance departure past midnight if needed
            if (depRaw >= 0) {
                int dayBase = (prevCum / 1440) * 1440;
                cumDep = dayBase + depRaw;
                if (cumDep < prevCum) cumDep += 1440;
                trainPrevCum[id] = cumDep;
            }
        }

        stops.push_back({id, name, station, cumArr, cumDep});
        allStations.insert(station);
    }

    // Build reverse index: station -> trains
    for (auto& [id, stops] : schedules) {
        set<string> seen;
        for (auto& s : stops) {
            if (seen.insert(s.station).second)
                stationIndex[s.station].push_back(id);
        }
    }
}

// ─── Routing ──────────────────────────────────────────────────────────────────

struct JourneyLeg {
    string trainId, trainName;
    string fromStation, toStation;
    int depTime, arrTime;   // absolute minutes (from user's chosen start-of-day)
};

// Info stored per Dijkstra node so we can reconstruct the full path.
struct PrevNode {
    int    arrTime;       // when we arrived at this station
    string prevStation;   // where the leg started
    string trainId;
    string trainName;
    int    boardTime;     // when the train departed prevStation
};

// Returns the optimal sequence of legs from `from` to `to`, or an empty vector
// if no route exists within 10 days of startTime.
vector<JourneyLeg> findRoute(const string& from, const string& to, int startTime) {
    const int MAX_TIME = startTime + 10 * 1440;

    map<string, int>      dist;   // station -> earliest absolute arrival
    map<string, PrevNode> prev;

    using P = pair<int, string>;
    priority_queue<P, vector<P>, greater<P>> pq;

    dist[from] = startTime;
    prev[from] = {startTime, "", "", "", -1};
    pq.push({startTime, from});

    while (!pq.empty()) {
        auto [t, s] = pq.top(); pq.pop();

        if (t > dist[s] || t > MAX_TIME) continue;
        if (s == to) break;

        auto it = stationIndex.find(s);
        if (it == stationIndex.end()) continue;

        for (const auto& trainId : it->second) {
            const auto& stops = schedules.at(trainId);
            if (stops.empty() || stops[0].cumDep < 0) continue;

            // T0 = first stop's departure (day-0 reference, minutes from midnight)
            int T0 = stops[0].cumDep;

            // Locate station s within this train's stop list
            for (int i = 0; i < (int)stops.size(); ++i) {
                if (stops[i].station != s) continue;
                if (stops[i].cumDep < 0) break; // terminus — can't board here

                // Offset of this stop's departure from T0
                int depOff = stops[i].cumDep - T0; // >= 0, since stops are ordered

                // Find k (number of days to advance) so the train departs at or after t.
                // depAbs = T0 + k*1440 + depOff  >=  t
                int k = 0;
                if (T0 + depOff < t)
                    k = (t - (T0 + depOff) + 1439) / 1440;
                int depAbs = T0 + k * 1440 + depOff;
                if (depAbs > MAX_TIME) break;

                // Board and ride to every subsequent stop
                for (int j = i + 1; j < (int)stops.size(); ++j) {
                    if (stops[j].cumArr < 0) continue;

                    int arrOff = stops[j].cumArr - T0; // >= depOff
                    int arrAbs = T0 + k * 1440 + arrOff;

                    const string& dest = stops[j].station;
                    auto dit = dist.find(dest);
                    if (dit == dist.end() || arrAbs < dit->second) {
                        dist[dest] = arrAbs;
                        prev[dest] = {arrAbs, s, trainId, stops[i].trainName, depAbs};
                        pq.push({arrAbs, dest});
                    }
                }
                break; // each train visits a station at most once (we found it)
            }
        }
    }

    if (!dist.count(to)) return {};

    // Reconstruct path by walking the prev-chain from destination to origin
    vector<JourneyLeg> legs;
    for (string cur = to; !prev[cur].prevStation.empty(); ) {
        const PrevNode& p = prev[cur];
        legs.push_back({p.trainId, p.trainName, p.prevStation, cur,
                        p.boardTime, p.arrTime});
        cur = p.prevStation;
    }
    reverse(legs.begin(), legs.end());
    return legs;
}

// ─── Display Helpers ──────────────────────────────────────────────────────────

static const string LINE60(60, '-');
static const string LINE60E(60, '=');

void printBanner() {
    cout << "\n";
    cout << "  +============================================================+\n";
    cout << "  |       INDIAN RAILWAY  MULTI-HOP  ROUTE FINDER             |\n";
    cout << "  |  Optimal connections across 250+ trains using Dijkstra    |\n";
    cout << "  +============================================================+\n\n";
}

void listStations() {
    cout << "\n  Available Stations (" << allStations.size() << " total):\n";
    cout << "  " << LINE60 << "\n  ";
    int col = 0;
    for (const auto& s : allStations) {
        cout << left << setw(18) << s;
        if (++col % 4 == 0) cout << "\n  ";
    }
    if (col % 4 != 0) cout << "\n";
    cout << "  " << LINE60 << "\n";
}

void printRoute(const vector<JourneyLeg>& legs,
                int startTime,
                const string& from, const string& to) {
    cout << "\n  " << LINE60E << "\n";

    if (legs.empty()) {
        cout << "  No route found from " << from << " to " << to
             << " within 10 days.\n  " << LINE60E << "\n";
        return;
    }

    int totalTime = legs.back().arrTime - startTime;

    cout << "  ROUTE  :  " << from << "  ->  " << to << "\n";
    cout << "  Journey:  " << fmtDur(totalTime) << "  |  "
         << legs.size() << " train(s)\n";
    cout << "  " << LINE60 << "\n";

    for (int i = 0; i < (int)legs.size(); ++i) {
        const auto& leg = legs[i];
        int waitFrom = (i == 0) ? startTime : legs[i - 1].arrTime;
        int wait     = leg.depTime - waitFrom;

        // Transfer / initial wait
        if (wait > 0) {
            string waitAt = (i == 0) ? from : legs[i - 1].toStation;
            if (i == 0) {
                cout << "\n  [*] Wait " << fmtDur(wait)
                     << " at " << waitAt
                     << "  (train departs " << fmtTime(leg.depTime) << ")\n";
            } else {
                cout << "\n  +---------  TRANSFER  ---------+\n";
                cout << "  | Station : " << left << setw(28) << waitAt    << "|\n";
                cout << "  | Wait    : " << left << setw(28) << fmtDur(wait) << "|\n";
                cout << "  | Board at: " << left << setw(28) << fmtTime(leg.depTime) << "|\n";
                cout << "  +------------------------------+\n";
            }
        }

        int onTrain = leg.arrTime - leg.depTime;

        cout << "\n  === TRAIN " << (i + 1) << " ===  "
             << leg.trainId << "  \"" << leg.trainName << "\"\n";
        cout << "  Depart : " << left << setw(20) << leg.fromStation
             << "  at  " << fmtTime(leg.depTime) << "\n";
        cout << "  Arrive : " << left << setw(20) << leg.toStation
             << "  at  " << fmtTime(leg.arrTime) << "\n";
        cout << "  On-board time: " << fmtDur(onTrain) << "\n";

        // Show intermediate stops along this leg
        const auto& stops = schedules.at(leg.trainId);
        int T0 = stops[0].cumDep;
        // Find which running (k) was used
        int k = (leg.depTime >= T0) ? (leg.depTime - T0) / 1440 : 0;
        bool inLeg = false;
        bool shownAny = false;
        for (const auto& st : stops) {
            if (st.station == leg.fromStation) { inLeg = true; continue; }
            if (!inLeg) continue;
            if (st.station == leg.toStation) break;
            if (!shownAny) {
                cout << "  Stops via :";
                shownAny = true;
            }
            int stopArr = T0 + k * 1440 + (st.cumArr - T0);
            cout << "  " << st.station << " (" << fmtTime(stopArr) << ")";
        }
        if (shownAny) cout << "\n";
    }

    cout << "\n  " << LINE60 << "\n";
    cout << "  ARRIVED at " << to
         << "  at  " << fmtTime(legs.back().arrTime) << "\n";
    cout << "  Total time from your departure: " << fmtDur(totalTime) << "\n";
    cout << "  " << LINE60E << "\n\n";
}

// ─── Data Generation ─────────────────────────────────────────────────────────
//
// Generates trains.csv with 250 trains across 25 corridors.
// Only runs when trains.csv does not already exist.

static string g2(int n) {                         // zero-pad to 2 digits
    return (n < 10 ? "0" : "") + to_string(n);
}
static string minsToHHMM(int m) {                 // raw minutes -> HH:MM string
    m = ((m % 1440) + 1440) % 1440;
    return g2(m / 60) + ":" + g2(m % 60);
}

struct Corridor {
    string namePrefix;
    int    idBase;           // first train's numeric ID
    int    numTrains;        // how many daily trains to generate
    int    firstDep;         // departure of first train (minutes from midnight)
    int    dwellMins;        // stop dwell time at intermediate stations
    vector<string> stations;
    vector<int>    legMins;  // travel time between consecutive stations
    bool   addReverse;       // also generate trains in the reverse direction
    Corridor(string np, int ib, int nt, int fd, int dm,
             vector<string> st, vector<int> lm, bool rev = false)
        : namePrefix(np), idBase(ib), numTrains(nt), firstDep(fd),
          dwellMins(dm), stations(st), legMins(lm), addReverse(rev) {}
};

void generateData(const string& fname) {
    // All 25 stations used across the network
    // Distances / travel times are realistic for Indian railways (approx).
    //
    // Key travel times (minutes):
    //   DEL-JAI 300, DEL-CHD 210, DEL-LUC 330, DEL-BHO 420, DEL-AMR 360,
    //   JAI-AHM 360, AHM-SUR 180, SUR-MUM 150, MUM-PUN 210, PUN-HYD 510,
    //   HYD-BAN 540, BAN-CHN 300, LUC-VAR 150, VAR-PAT 210, PAT-KOL 450,
    //   BHO-NAG 270, NAG-PUN 510, KOL-BHU 300, BHU-VIZ 300, VIZ-CHN 780,
    //   CHN-CBE 300, CBE-KOC 150, CHN-MAD 420, MAD-KOC 180, KOL-GUW 660,
    //   NAG-HYD 420, IND-BHO 150, JAI-IND 300, PAT-RAN 240, RAN-BHU 270,
    //   CHD-AMR 150, LUC-CHD 420, HYD-VIZ 360, BAN-KOC 480, CBE-BAN 300,
    //   MAD-CBE 180, VAR-RAN 360

    // Constructor args: namePrefix, idBase, numTrains, firstDep, dwellMins,
    //                   stations, legMins, addReverse
    // addReverse=true generates trains in both directions (doubles count).
    vector<Corridor> corridors = {
        // ── Trunk routes (both directions) ──────────────────────────────────
        Corridor("Deccan Express",     12001, 8, 360, 15,
         {"DELHI","BHOPAL","NAGPUR","PUNE","MUMBAI"},          {420,270,510,240}, true),

        Corridor("Saurashtra Mail",    12051, 8, 420, 15,
         {"DELHI","JAIPUR","AHMEDABAD","SURAT","MUMBAI"},      {300,360,180,150}, true),

        Corridor("Poorva Express",     12301, 8, 480, 15,
         {"DELHI","LUCKNOW","VARANASI","PATNA","KOLKATA"},     {330,150,210,450}, true),

        Corridor("GT Express",         12615, 8, 300, 15,
         {"DELHI","NAGPUR","HYDERABAD","BANGALORE","CHENNAI"}, {690,420,540,300}, true),

        Corridor("Konkan Kanya",       12779, 8, 300, 15,
         {"MUMBAI","PUNE","HYDERABAD","BANGALORE"},            {210,510,540},     true),

        Corridor("Tamil Nadu Express", 12621, 8, 600, 15,
         {"BANGALORE","CHENNAI"},                              {300},             true),

        Corridor("East Coast Express", 12840, 8, 360, 20,
         {"MUMBAI","PUNE","HYDERABAD","VISAKHAPATNAM","BHUBANESWAR","KOLKATA"},
                                                               {210,510,360,300,300}, true),

        Corridor("Coromandel Express", 12841, 8, 420, 15,
         {"KOLKATA","BHUBANESWAR","VISAKHAPATNAM","CHENNAI"},  {300,300,780},     true),

        Corridor("Shatabdi Express",   12011, 8, 360, 10,
         {"DELHI","CHANDIGARH","AMRITSAR"},                    {210,150},         true),

        Corridor("Ganga Yamuna",       14005, 8, 480, 15,
         {"DELHI","LUCKNOW","VARANASI"},                       {330,150},         true),

        Corridor("Island Express",     16331, 8, 540, 15,
         {"CHENNAI","COIMBATORE","KOCHI"},                     {300,150},         true),

        Corridor("Kamrupa Express",    15901, 8, 420, 20,
         {"KOLKATA","GUWAHATI"},                               {660},             true),

        // ── Medium regional (both directions) ───────────────────────────────
        Corridor("Ajanta Express",     11201, 6, 300, 15,
         {"BHOPAL","NAGPUR","HYDERABAD"},                      {270,420},         true),

        Corridor("Steel Express",      12869, 6, 360, 15,
         {"PATNA","RANCHI","BHUBANESWAR"},                     {240,270},         true),

        Corridor("Saryu Yamuna",       15003, 6, 420, 10,
         {"LUCKNOW","VARANASI","PATNA"},                       {150,210},         true),

        Corridor("Saurashtra Express", 19005, 6, 300, 15,
         {"AHMEDABAD","SURAT","MUMBAI"},                       {180,150},         true),

        Corridor("Rajiv Rajeswari",    17229, 6, 480, 15,
         {"HYDERABAD","BANGALORE"},                            {540},             true),

        Corridor("Utkal Express",      18477, 6, 360, 15,
         {"KOLKATA","BHUBANESWAR"},                            {300},             true),

        Corridor("Visakha Express",    18519, 6, 420, 15,
         {"BHUBANESWAR","VISAKHAPATNAM"},                      {300},             true),

        Corridor("Nagercoil Express",  16188, 6, 360, 15,
         {"CHENNAI","MADURAI","KOCHI"},                        {420,180},         true),

        Corridor("Kanyakumari Exp",    16526, 6, 420, 15,
         {"COIMBATORE","BANGALORE"},                           {300},             true),

        Corridor("Vidarbha Express",   12105, 6, 300, 15,
         {"NAGPUR","BHUBANESWAR","VISAKHAPATNAM"},             {540,300},         true),

        Corridor("Malwa Express",      12919, 6, 480, 15,
         {"INDORE","BHOPAL","NAGPUR"},                         {150,270},         true),

        Corridor("Amritsar Express",   14673, 6, 360, 10,
         {"DELHI","AMRITSAR"},                                 {360},             true),

        // ── Short connectors ─────────────────────────────────────────────────
        Corridor("Pink City Express",  12985, 6, 300, 10,
         {"DELHI","JAIPUR"},                                   {300},             true),

        Corridor("Deccan Queen",       12123, 6, 480, 10,
         {"MUMBAI","PUNE"},                                    {210},             true),

        Corridor("Kashi Vishwanath",   15127, 5, 300, 10,
         {"VARANASI","RANCHI","BHUBANESWAR"},                  {360,270},         true),

        Corridor("Rajasthan Express",  19713, 5, 420, 15,
         {"JAIPUR","INDORE","BHOPAL"},                         {300,150},         true),

        Corridor("Narmada Express",    11463, 5, 480, 15,
         {"AHMEDABAD","INDORE","BHOPAL"},                      {210,150},         true),

        Corridor("Sinhagad Express",   11009, 5, 420, 10,
         {"PUNE","NAGPUR"},                                    {300},             true),

        Corridor("Godavari Express",   12727, 5, 360, 15,
         {"HYDERABAD","VISAKHAPATNAM"},                        {360},             true),

        Corridor("Parasuram Express",  16650, 5, 480, 15,
         {"BANGALORE","KOCHI"},                                {480},             true),

        Corridor("Pandian Express",    12637, 5, 360, 10,
         {"MADURAI","COIMBATORE"},                             {180},             true),

        Corridor("Saraighat Express",  15961, 5, 540, 20,
         {"PATNA","GUWAHATI"},                                 {600},             true),

        Corridor("Himalayan Queen",    14095, 5, 300, 10,
         {"CHANDIGARH","LUCKNOW"},                             {420},             true),

        Corridor("Mahanagari Express", 11093, 5, 480, 15,
         {"LUCKNOW","PATNA","RANCHI"},                         {360,240},         true),
    };

    ofstream out(fname);
    if (!out.is_open()) {
        cerr << "ERROR: cannot write to " << fname << "\n";
        exit(1);
    }

    out << "train_id,train_name,station,arrival,departure\n";

    // Helper lambda: write one corridor's trains to the CSV
    auto writeCorridor = [&](const vector<string>& stations,
                             const vector<int>&    legs,
                             int dwellMins, int numTrains,
                             int firstDep, int idBase,
                             const string& namePrefix,
                             int& total) {
        int interval = 1440 / numTrains;
        for (int i = 0; i < numTrains; ++i) {
            int tid = idBase + i * 2;
            string nm = namePrefix + " " + to_string(i + 1);
            int cur = (firstDep + i * interval) % 1440;

            out << tid << "," << nm << "," << stations[0]
                << ",--," << minsToHHMM(cur) << "\n";

            for (int j = 1; j < (int)stations.size(); ++j) {
                cur += legs[j - 1];
                string arr = minsToHHMM(cur);
                if (j == (int)stations.size() - 1) {
                    out << tid << "," << nm << "," << stations[j]
                        << "," << arr << ",--\n";
                } else {
                    string dep = minsToHHMM(cur + dwellMins);
                    out << tid << "," << nm << "," << stations[j]
                        << "," << arr << "," << dep << "\n";
                    cur += dwellMins;
                }
            }
            ++total;
        }
    };

    int totalGenerated = 0;
    for (const auto& c : corridors) {
        // Forward direction
        writeCorridor(c.stations, c.legMins, c.dwellMins, c.numTrains,
                      c.firstDep, c.idBase, c.namePrefix, totalGenerated);

        // Reverse direction (offset departure by ~9h to avoid identical slots)
        if (c.addReverse) {
            vector<string> revSt(c.stations.rbegin(), c.stations.rend());
            vector<int>    revLg(c.legMins.rbegin(),  c.legMins.rend());
            // Reverse trains use idBase in the 90000s range to avoid conflicts
            int revBase    = 90000 + c.idBase;
            int revNumTr   = max(4, c.numTrains);
            int revFirstDp = (c.firstDep + 540) % 1440; // offset by ~9 h
            string revName = c.namePrefix + " Ret";
            writeCorridor(revSt, revLg, c.dwellMins, revNumTr,
                          revFirstDp, revBase, revName, totalGenerated);
        }
    }

    cout << "  Generated " << totalGenerated << " trains -> " << fname << "\n";
}

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    string dataFile = (argc > 1) ? argv[1] : "trains.csv";

    printBanner();

    // Auto-generate sample data if file is missing
    {
        ifstream probe(dataFile);
        if (!probe.is_open()) {
            cout << "  trains.csv not found — generating sample dataset...\n";
            generateData(dataFile);
        }
    }

    loadCSV(dataFile);

    cout << "  Loaded " << schedules.size() << " trains across "
         << allStations.size() << " stations.\n";

    listStations();

    // ── Interactive loop ─────────────────────────────────────────────────────
    while (true) {
        cout << "\n  " << LINE60 << "\n";
        cout << "  From station  (or 'list' to show stations, 'quit' to exit): ";
        string from;
        if (!getline(cin, from)) break;

        if (from == "quit" || from == "q" || from == "exit") break;
        if (from == "list" || from == "l") { listStations(); continue; }

        // Normalise to upper-case
        transform(from.begin(), from.end(), from.begin(), ::toupper);

        if (!allStations.count(from)) {
            cout << "  Station \"" << from
                 << "\" not found. Type 'list' to see all stations.\n";
            continue;
        }

        cout << "  To station  : ";
        string to;
        if (!getline(cin, to)) break;
        transform(to.begin(), to.end(), to.begin(), ::toupper);

        if (!allStations.count(to)) {
            cout << "  Station \"" << to << "\" not found.\n";
            continue;
        }
        if (from == to) {
            cout << "  Departure and destination are the same station.\n";
            continue;
        }

        cout << "  Departure time (HH:MM, 24-hr, e.g. 09:30): ";
        string ts;
        if (!getline(cin, ts)) break;
        int startTime = parseTime(ts);
        if (startTime < 0) {
            cout << "  Invalid time format — use HH:MM (e.g. 07:45)\n";
            continue;
        }

        cout << "\n  Searching fastest route...";
        cout.flush();
        auto route = findRoute(from, to, startTime);
        printRoute(route, startTime, from, to);
    }

    cout << "\n  Thank you for using the Train Route Finder!\n\n";
    return 0;
}