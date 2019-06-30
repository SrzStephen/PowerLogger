#include <vector>
#include <Arduino.h>
#include <string>
class DataClass
{
    std::string program_version;
    std::string program_compiled;
    std::string device_identifier;
    int condense_frequency;
    public:
        DataClass(std::string program_version, std::string program_compiled,std::string device_identifier,int condense_frequency);
        void add_pulse(long timestamp);
        std::vector<long> return_pulses();
        void remove_sent_pulses();
        int items = 0;
        std::string return_post_payload();
        
        

    private:
        std::vector<long> data_vector;
        std::vector<long> snapshot_vector;
        SemaphoreHandle_t data_lock;
        std::string payload_preamble;
        
        
};