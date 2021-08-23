#pragma once

#include <chrono>
#include <iostream>


#define PROFILE_CONCAT_INTERNAL(X, Y) X##Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x,s) LogDuration UNIQUE_VAR_NAME_PROFILE(x,s)

class LogDuration {
public:
    
    using Clock = std::chrono::steady_clock;
    
    LogDuration(const std::string& id) : id_(id),stream_(std::cerr)  {
    }
    LogDuration(std::ostream& stream) : id_("Operation time"),stream_(std::cout)  {
    }
    LogDuration(const std::string& id, std::ostream& stream) : stream_(stream)  {
        using namespace std;
        if (&stream_ == &cout){
            id_ = "Operation time"s;
        } else {
            id_=id;
        }
    }
    
    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;
        
        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        stream_ << id_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }
    
private:
    std::string id_;
    std::ostream& stream_;
    const Clock::time_point start_time_ = Clock::now();
};
