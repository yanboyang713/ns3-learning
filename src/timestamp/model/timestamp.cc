#include "timestamp.h"
#include "ns3/core-module.h"
#include <iostream>

timestamp::timestamp(){
    timeStampGenerate();
}

timestamp::~timestamp(){

}
std::string timestamp::padLeadingZeros(std::string old_str, size_t width){
    return std::string(width - std::min(width, old_str.length()), '0') + old_str;
}

std::string timestamp::padLeadingZeros(int number, int width){
    std::ostringstream ss;
    ss << std::setw(width) << std::setfill('0') << number;
    return ss.str();
}

std::string timestamp::getTimeString () const{
    return time.timeString;
}

void timestamp::timeStampGenerate(){

    //std::cout << "Micro Seconds: "<< Simulator::Now().GetMicroSeconds() << std::endl;

    ns3::Time SimulatorCurrentTime = ns3::Simulator::Now();

    //std::string micro = std::to_string(ns3::Simulator::Now().GetMicroSeconds());
    std::string micro = std::to_string(SimulatorCurrentTime.GetMicroSeconds());
    if (micro.length() < 6){
        micro =  padLeadingZeros(micro, 6);
    }
    else if (micro.length() > 6){
        micro = micro.substr(micro.length() - 6);
    }
    time.micro = micro;

    //std::cout << "Seconds: "<< Simulator::Now().GetSeconds() << std::endl;
    //std::string sec = std::to_string(floor(ns3::Simulator::Now().GetSeconds()));
    std::string sec = std::to_string(floor(SimulatorCurrentTime.GetSeconds()));
    if (sec.length() < 2){
        sec = padLeadingZeros(sec, 2);
    }
    else if (sec.length() > 2){
        sec = sec.substr(sec.length() - 2);
    }
    time.seconds = sec;

    //std::cout << "Minutes: "<< Simulator::Now().GetMinutes() << std::endl;
    //std::string min = std::to_string(floor(ns3::Simulator::Now().GetMinutes()));
    std::string min = std::to_string(floor(SimulatorCurrentTime.GetMinutes()));
    if (min.length() < 2){
        min = padLeadingZeros(min, 2);
    }
    else if (min.length() > 2){
        min = min.substr(min.length() - 2);
    }
    time.minutes = min;

    //std::cout << "Hours: "<< Simulator::Now().GetHours() << std::endl;
    //std::string hour = std::to_string(floor(ns3::Simulator::Now().GetHours()));
    std::string hour = std::to_string(floor(SimulatorCurrentTime.GetHours()));
    if (hour.length() < 2){
        hour = padLeadingZeros(hour, 2);
    }
    else if (hour.length() > 2){
        hour = hour.substr(hour.length() - 2);
    }
    time.hours = hour;

    // Get the current time
    auto now = std::chrono::system_clock::now();

    // Convert to time_t, which represents the number of seconds since 1970-01-01 00:00:00 UTC
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

    // Convert to a local time
    std::tm* local_time = std::localtime(&currentTime);

    // Get the year, month, and day from the local time
    int year = local_time->tm_year + 1900;
    int month = local_time->tm_mon + 1;
    int day = local_time->tm_mday;

    time.year = std::to_string (year);
    time.month = padLeadingZeros(month, 2);
    time.day = padLeadingZeros(day, 2);

    //Time String
    time.timeString = time.year + "-" + time.month + "-" + time.day + " " + time.hours + ":" + time.minutes + ":" + time.seconds + "." + time.micro;
    //std::cout << "time string: " << time.timeString << std::endl;

    return;
}
