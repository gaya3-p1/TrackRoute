# midrail
an easy way to find the earliest possible way to travel from two stations which doesn't have direct railway option.

I generate a csv file, trains.csv consists of **400+** virtual trains which passing through **25** stations.

Available Stations:  
AHMEDABAD, AMRITSAR, BANGALORE, BHOPAL, BHUBANESWAR, CHANDIGARH, CHENNAI, COIMBATORE, DELHI, GUWAHATI, HYDERABAD, INDORE, JAIPUR, KOCHI, KOLKATA, LUCKNOW, MADURAI, MUMBAI, NAGPUR, PATNA, PUNE, RANCHI, SURAT, VARANASI and VISAKHAPATNAM.  

-- to use the mid rails --  
g++ -std=c++17 -o route_finder main.cpp  
./route_finder trains.csv
