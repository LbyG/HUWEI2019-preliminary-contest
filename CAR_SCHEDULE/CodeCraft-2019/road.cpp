#include "road.h"
#include "util.h"

#include <vector>
#include <iostream>

using namespace std;

road::road(string road_info) {
    // road_info = (id,length,speed,channel,from,to,isDuplex)
    vector<int> info_val = parse_string_to_int_vector(road_info);
    if (info_val.size() != 7) {
        cout << "road::road error!!!!!!!!!!!!!!!!!" << endl;
    }
    this->id = info_val[0];
    this->length = info_val[1];
    this->speed = info_val[2];
    this->channel = info_val[3];
    this->from = info_val[4];
    this->to = info_val[5];
    this->is_duplex = info_val[6];
    
    if (this->speed > this->length) {
        cout << "road::road error!!!!!!!!!!!!!!!!!" << endl;
    }
    
    // road.into_channel_id = 0
    this->set_into_channel_id(0);
    // road.wait_road_direction_count = [0, 0, 0]
    this->init_wait_into_road_direction_count();
    // road.wait_car_forefront_of_each_channel.clear();
    this->clear_wait_car_forefront_of_each_channel();
    
    // init cars_in_road
    this->cars_in_road.clear();
    for (int i = 0; i < channel; i ++) {
        this->cars_in_road.push_back(list<car>());
    }
}

// return road id
int road::get_id() const {
    return this->id;
}

// return road from cross id
int road::get_from() const {
    return this->from;
}

// return road to cross id
int road::get_to() const {
    return this->to;
}

// return is_duplex
int road::get_is_duplex() const {
    return this->is_duplex;
}

// swap from and to
void road::swap_from_to() {
    int tmp = this->from;
    this->from = this->to;
    this->to = tmp;
}

// set.into_channel_id = into_channel_id
void road::set_into_channel_id(int into_channel_id) {
    this->into_channel_id = into_channel_id;
}

// road.wait_road_direction_count = [0, 0, 0]
void road::init_wait_into_road_direction_count() {
    this->wait_into_road_direction_count = vector<int>(3, 0);
}

// road.wait_car_forefront_of_each_channel.clear();
void road::clear_wait_car_forefront_of_each_channel() {
    clear_priority_queue(this->wait_car_forefront_of_each_channel);
}

// if no car need through cross in this road
bool road::if_no_car_through_cross() {
    return this->wait_car_forefront_of_each_channel.empty();
}

// get car which have priority through cross, wait_car_forefront_of_each_channel
car road::get_car_priority_through_cross() {
    return this->wait_car_forefront_of_each_channel.top();
}

// have car through cross
void road::have_car_through_cross(int channel_id) {
    if (this->cars_in_road[channel_id].front().get_dis_to_cross() != this->wait_car_forefront_of_each_channel.top().get_dis_to_cross()) {
        cout << "road::have_car_through_cross error !!!!!!!!!!!!!!!!!!!!" << endl;
    }
    if (channel_id != this->wait_car_forefront_of_each_channel.top().get_channel_id()) {
        cout << "road::have_car_through_cross error !!!!!!!!!!!!!!!!!!!!" << endl;
    }
    this->cars_in_road[channel_id].pop_front();
    this->wait_car_forefront_of_each_channel.pop();
}

// wait_into_road_direction_count[car_direct] += 1
void road::add_wait_into_road_direction_count(int car_direct) {
    this->wait_into_road_direction_count[car_direct] += 1;
}
// wait_into_road_direction_count[car_direct] -= 1
void road::sub_wait_into_road_direction_count(int car_direct) {
    this->wait_into_road_direction_count[car_direct] -= 1;
}

// check whether car_turn_direct can enter this road
bool road::check_direct_priority(int car_turn_direct) {
    for (int i = 0; i < car_turn_direct; i ++)
        if (this->wait_into_road_direction_count[i] > 0)
            return false;
        else if (this->wait_into_road_direction_count[i] < 0 || this->wait_into_road_direction_count[i] > 1) {
            cout << "road id = " << this->id << " from = " << this->from << " to = " << this->to << endl;
            cout << "road::check_direct_priority error !!!!!!!!!!!!!!!!!!" << endl;
        }
    return true;
}

// init cars' schedule state to wait state which in channel
// return number of cars in channel
int road::init_cars_schedule_status_in_channel(int channel_id) {
    int cars_in_road_n = 0;
    for (list<car>::iterator iter = this->cars_in_road[channel_id].begin(); iter != this->cars_in_road[channel_id].end(); iter ++) {
        iter->set_schedule_status(1);
        cars_in_road_n ++;
    }
    return cars_in_road_n;
}

// Schedule cars in road.
// If car can through cross then car into schedule wait -> car.schedule_status = 1
// If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
// If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 0
// If car blocked by termination state car then car move to the back of the previous car -> car.schedule_status = 0
// return number of from wait state to termination status
int road::schedule_cars_running_in_channel(int channel_id, int flag, int &cars_running_n, int &cars_arrive_destination_n, int &all_cars_running_time, int T) {
    int cars_running_termination_status_n = 0;
    
    // car arrive destination direct drive to destination
    if (this->cars_in_road[channel_id].empty())
        return cars_running_termination_status_n;
    list<car>::iterator iter = this->cars_in_road[channel_id].begin();
    int car_dis_to_cross = iter->get_dis_to_cross();
    int speed_car_in_road = min(this->speed, iter->get_speed());
    if (iter->get_schedule_status() == 1) {
        // if forefront car is terminate state then don't to schedule
        // because have forefront because next road is fiil up so termination remain in cross
        // deal forefront car in channel
        if (car_dis_to_cross >= speed_car_in_road) {
            // forefront car can't through cross -> drive and to termination state
            iter->set_dis_to_cross(car_dis_to_cross - speed_car_in_road);
            iter->set_schedule_status(0);
            cars_running_termination_status_n ++;
        } else {
            // forefront car can through cross -> to wait state
            this->wait_car_forefront_of_each_channel.push(*iter);
            iter->set_schedule_status(1);
        }
    }
    list<car>::iterator previous_iter = iter;
    int previous_car_dis_to_cross = previous_iter->get_dis_to_cross();
    for (iter ++; iter != this->cars_in_road[channel_id].end() && iter->get_schedule_status() == 1; iter ++) {
        car_dis_to_cross = iter->get_dis_to_cross();
        int car_dis_to_previous_car = car_dis_to_cross - previous_car_dis_to_cross - 1;
        speed_car_in_road = min(this->speed, iter->get_speed());
        if (car_dis_to_previous_car >= speed_car_in_road) {
            // have enough dis for car drive
            iter->set_dis_to_cross(car_dis_to_cross - speed_car_in_road);
            iter->set_schedule_status(0);
            cars_running_termination_status_n ++;
        } else {
            // don't have enough dis for car drive
            if (previous_iter->get_schedule_status() == 0) {
                // previous car is termination state
                iter->set_dis_to_cross(previous_car_dis_to_cross + 1);
                if (iter->get_dis_to_cross() >= this->length) {
                    cout << "road::schedule_cars_running_in_channel error!!!!!!!!!!!!!!!!" << endl;
                }
                iter->set_schedule_status(0);
                cars_running_termination_status_n ++;
            } else {
                // previous car is wait state
                iter->set_schedule_status(1);
            }
        }
        previous_iter = iter;
        previous_car_dis_to_cross = previous_iter->get_dis_to_cross();
    }
    return cars_running_termination_status_n ;
}

int road::schedule_cars_running_in_road(int &cars_running_n, int &cars_arrive_destination_n, int &all_cars_running_time, int T) {
    int cars_running_wait_status_n = 0;
    // road.into_channel_id = 0
    this->set_into_channel_id(0);
    // road.wait_road_direction_count = [0, 0, 0]
    this->init_wait_into_road_direction_count();
    // road.wait_car_forefront_of_each_channel.clear();
    this->clear_wait_car_forefront_of_each_channel();
    // drive all car in channel id
    for (int channel_id = 0; channel_id < channel; channel_id ++) {
        cars_running_wait_status_n += this->init_cars_schedule_status_in_channel(channel_id);
        cars_running_wait_status_n -= this->schedule_cars_running_in_channel(channel_id, 1, cars_running_n, cars_arrive_destination_n, all_cars_running_time, T);
    }
    return cars_running_wait_status_n;
}

// forefront car because some reason it need remain in cross
int road::forefront_car_remain_in_cross(int channel_id, int flag, int &cars_running_n, int &cars_arrive_destination_n, int &all_cars_running_time, int T) {
    int wait_to_termination_n = 1;
    this->wait_car_forefront_of_each_channel.pop();
    this->cars_in_road[channel_id].front().set_schedule_status(0);
    this->cars_in_road[channel_id].front().set_dis_to_cross(0);
    wait_to_termination_n += this->schedule_cars_running_in_channel(channel_id, 1, cars_running_n, cars_arrive_destination_n, all_cars_running_time, T);
    return wait_to_termination_n;
}

// check the road all channel whether be fill up
bool road::whether_be_fill_up() {
    if (this->into_channel_id == this->channel)
        return true;
    while (!this->cars_in_road[this->into_channel_id].empty() && this->cars_in_road[this->into_channel_id].back().get_dis_to_cross() == (this->length - 1) && this->cars_in_road[this->into_channel_id].back().get_schedule_status() == 0) {
        this->into_channel_id ++;
        if (this->into_channel_id == this->channel)
            return true;
    }
    // have a channel can enter
    return false;
}

// if car speed_in_road <= car.dis_to_cross in previous road, then dis_move_in_road = 0 -> car don't enter this road, car.dis_to_cross = 0 and return -1 
// else if car into road don't be block or block by a car which is termination status, car enter road, return 1
// else car can't enter road, need wait previous car to be termination state return 0
int road::car_into_road(car into_car, int flag) {
    int speed_car_in_road = min(this->speed, into_car.get_speed());
    int dis_move_in_road = speed_car_in_road - into_car.get_dis_to_cross();
    if (dis_move_in_road <= 0)
        return -1;
    int car_dis_to_cross = this->length - dis_move_in_road;
    if (this->cars_in_road[this->into_channel_id].empty() || (car_dis_to_cross > this->cars_in_road[this->into_channel_id].back().get_dis_to_cross())) {
        if (flag == 0)
            return 1;
        if (into_car.get_next_road_in_path() == -1) {
            cout << "road::car_into_road error!!!!!!!!!!!!!!!!!!!" << endl;
            return 1;
        }
        // don't be block
        // update car state
        into_car.arrive_next_road_path();
        into_car.set_schedule_status(0);
        into_car.set_dis_to_cross(car_dis_to_cross);
        if (into_car.get_dis_to_cross() >= this->length) {
            cout << "road::car_into_road error!!!!!!!!!!!!!!!!" << endl;
        }
        into_car.set_channel_id(this->into_channel_id);
        this->cars_in_road[this->into_channel_id].push_back(into_car);
        return 1;
    } else {
        if (this->cars_in_road[this->into_channel_id].back().get_schedule_status() == 0) {
            if (flag == 0)
                return 1;
            if (into_car.get_next_road_in_path() == -1) {
                cout << "road::car_into_road error!!!!!!!!!!!!!!!!!!!" << endl;
                return 1;
            }
            // block by a car which is termination status
            // update car state
            into_car.arrive_next_road_path();
            into_car.set_schedule_status(0);
            into_car.set_dis_to_cross(this->cars_in_road[this->into_channel_id].back().get_dis_to_cross() + 1);
            if (into_car.get_dis_to_cross() >= this->length) {
                cout << "road::car_into_road error!!!!!!!!!!!!!!!!" << endl;
            }
            into_car.set_channel_id(this->into_channel_id);
            this->cars_in_road[this->into_channel_id].push_back(into_car);
            return 1;
        } else {
            // lock by a car which is wait status, need wait too
            return 0;
        }
    }
    cout << "road::car_into_road error!!!!!!!!!!!!!!!!" << endl;
    return 0;
}

// output road status
void road::output_status(int T) {
    cout << "road id = " << this->id << " from = " << this->from << " to = " << this->to << " length = " << this->length << " speed = " << this->speed << endl;
    for (int i = 0; i < this->channel; i ++) {
        if (this->cars_in_road[i].size() > 0) {
            cout << "   channel id = " << i << ":";
            for (list<car>::iterator iter = this->cars_in_road[i].begin(); iter != this->cars_in_road[i].end(); iter ++) {
                cout << T << "(" << iter->get_id() << "," << iter->get_channel_id() << "," << iter->get_dis_to_cross() << "," << iter->get_speed() << "," << this->id << ")";
           }
            cout << endl;
        }
    }
}

// Overload < for road by id
bool operator<(const road &a, const road &b) {
    return a.get_id() < b.get_id();
}