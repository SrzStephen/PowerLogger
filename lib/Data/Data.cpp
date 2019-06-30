#include <Data.h>
#include <algorithm>
#include <Arduino.h>
#include <string>
#include <sstream>
#include <iostream>
#include <cstring>
//basic data class to deal with adding/removing data easily without dealing with annoying parts of sharing data between both esp32 cores.
DataClass::DataClass(std::string program_version, std::string program_compiled,std::string device_identifier,int condense_frequency_)
{
    data_lock = xSemaphoreCreateMutex();
    payload_preamble = "energy_meter_reading,device=" + device_identifier + "," + "version=" + program_version + "," + "compiled=" + program_compiled + " value=";
    condense_frequency = condense_frequency_;
}
void DataClass::add_pulse(long timestamp)
{
    xSemaphoreTake(data_lock,portMAX_DELAY);
    data_vector.push_back(timestamp);
    xSemaphoreGive(data_lock);
    items = data_vector.size();
}
// when sending a post request I create a variable snapshot_vector with a snapshot of all the items I sent
// avoid sending this data twice by calling remove_sent_pulses after a successful post request.
void DataClass::remove_sent_pulses()
{
    xSemaphoreTake(data_lock,portMAX_DELAY);
    for (auto &value : snapshot_vector)
    {
        data_vector.erase(std::find(data_vector.begin(),data_vector.end(),value));
    }
    xSemaphoreGive(data_lock);
    items = data_vector.size();
}

std::vector<long> DataClass::return_pulses()
{
    return data_vector;
}

std::vector<long> get_data_snapshot()
{
}
std::string DataClass::return_post_payload()
{
    // Constructs an infuxdb post request to send data.
    std::string payload;
    
    //create a frozen snapshot of data
    xSemaphoreTake(data_lock,portMAX_DELAY);
    snapshot_vector = data_vector;
    xSemaphoreGive(data_lock);
    // condense the data so I'm not sending a stupid amount of data in the post request
    long first_timestamp = snapshot_vector[0];
    long last_timestamp = first_timestamp + condense_frequency;
    int count=1;
    for (auto &value : snapshot_vector)
    {
        if (value <= last_timestamp)
        {
            count++;
        }
        else
        {
            count++;
            std::stringstream ss_count;
            std::stringstream ss_timestamp;
            ss_count << count;
            ss_timestamp << last_timestamp;
            payload = payload + payload_preamble + ss_count.str() + " " + ss_timestamp.str() + " ";
            count =0;
            first_timestamp = value;
            last_timestamp = first_timestamp+condense_frequency;
        }
    }
    //deal with the last bit of data if it exists
    if (first_timestamp != snapshot_vector[snapshot_vector.size()])
    {
        std::stringstream ss_count;
        std::stringstream ss_timestamp;
        ss_count << count;
        ss_timestamp << last_timestamp;
        payload = payload + payload_preamble + ss_count.str() + " " + ss_timestamp.str() + " ";
    }
     return payload;
}


