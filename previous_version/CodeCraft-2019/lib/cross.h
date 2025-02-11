#ifndef CROSS_H
#define CROSS_H

#include "road.h"

#include <map>
#include <vector>

using namespace std;

class cross {
private:
    int id; // cross id
    vector<int> road_id_in_cross; //road id in cross
    // turn_direct[road_id1][road_id2] = 0 road_id1 -> road_id2 Straight;
    // turn_direct[road_id1][road_id2] = 1 road_id1 -> road_id2 Turn left;
    // turn_direct[road_id1][road_id2] = 2 road_id1 -> road_id2 Turn right;
    map<int, map<int, int>> turn_direct;
    // situation of road can arrive this cross, road_in_cross[i].id < road_in_cross[i+1].id
    vector<road*> roads_into_cross;
    // road_departure_cross[road_id] -> road which id == road_id and from this cross to other cross
    map<int, road*> roads_departure_cross;
public:
    cross();
    // convert cross_info = (id,roadId1,roadId2,roadId3,roadId4) - > 
    // turn_direct[roadId1][roadId2] = 1     turn left 
    // turn_direct[roadId1][roadId3] = 0     straight
    // turn_direct[roadId1][roadId3] = 2     turn right
    // ...
    cross(string cross_info);
    // return cross id
    int get_id() const; 
    // query in this cross from from_road_id to to_road_id is 0->straight? 1->left? or 2->right?
    int get_turn_direct(int from_road_id, int to_road_id);
    // add road* to road_into_cross which road->to = cross_id
    void add_road_into_cross(road* road_pointer);
    // add road* to road_departure_cross which road->from == cross_id
    void add_road_departure_cross(int road_id, road* road_pointer);
    // return roads_departure_cross
    map<int, road*> get_roads_departure_cross();
    
    // update road state in cross
    void update_road_state_in_cross(road* road_pointer);
    void update_road_state_in_cross();
    void roads_into_cross_sort_by_id();
    
    // car to next road, maybe return -2, -1, 0, 1
    // if road is be termination status cars fill up return -2
    // else if car speed_in_road <= car.dis_to_cross in previous road, then dis_move_in_road = 0 -> car don't enter this road, car.dis_to_cross = 0 and return -1 
    // else if car into road don't be block or block by a car which is termination status, car enter road, return 1
    // else car can't enter road, need wait previous car to be termination state return 0
    int car_to_next_road(car car_through_cross);
    // schedule cars can through cross
    // return number of cars from wait status to termination status in this schedule
    int schedule_cars_in_cross(int &cars_running_n, int &cars_arrive_destination_n, int &all_cars_running_time, int T);
};

#endif