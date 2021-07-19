#include "stream/WebsocketEndpoint.h"
#include "PriceStream.h"
#include <nlohmann/json.hpp>
#include  <iostream>


#include <filesystem>
int main(int argc, char ** argv){

    spdlog::set_level(spdlog::level::debug);
    std::cout<<"Hello\n";
    // using namespace bot::stream::websocket;
    // websocket_endpoint endpoint;

    std::string base_uri_testnet = "wss://testnet.binance.vision/";
    std::string base_uri = "wss://stream.binance.com:9443/";
    std::string stream_location = "ws/!ticker@arr";
    std::string mini_ticker_stream_location = "ws/!miniTicker@arr";



    auto liveBot = bot::stream::live::LiveBot::getInstance(base_uri + stream_location,"storage/db");

    liveBot->start();

    while(1){
        std::string s;
        std::getline(std::cin,s);
        if(s == "q") break;
    }

    liveBot->stop();

    while(1){
        std::string s;
        std::getline(std::cin,s);
        if(s == "q") break;
    }

    // std::cout << argv[0] << " " << file_size("main.cpp") << '\n';









    return 0;
}