#include "PriceStream.h"
#include "FileIo.h"
#include "stream/WebsocketEndpoint.h"
#include "Asset.h"
#include "ds/TrieSet.h"
#include <nlohmann/json.hpp>
#include <cfloat>
#include <unordered_map>
#include <unordered_set>
#include <iostream>


#define FIAT_COIN_DB_FILE                       "_fiatcoin"
#define CRYPTO_COIN_DB_FILE                     "_cryptocoin"
#define ALL_COIN_PAIR_DB_FILE                   "_allcoinpair"
#define COIN_PRICE_DB_FILE_SUFFIX               "_coinpair_prices"

// #define DEFAULT_MAIN_LOOP_INTERVAL_MILISEC      5*60*1000            //intended value
#define DEFAULT_MAIN_LOOP_INTERVAL_MILISEC      10*1000            //testing value


#define NOW std::chrono::system_clock::now()
#define EPOCH std::chrono::system_clock::now().time_since_epoch()
#define EPOCH_SEC std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define EPOCH_MILISEC std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define EPOCH_NANOSEC std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count()


std::string to_str(const char * c_str){
    std::string str(c_str);
    return str;
}

namespace bot
{


    
    namespace stream
    {
        namespace live
        {
            template<typename _real> _real toDouble(std::string num){
                bool neg = false;
                if(num[0] == '-'){
                    neg = true;
                    num = num.substr(1);
                }
                _real before_decimal = 0.0;
                int i = 0;
                for(; i < num.size() and num[i] != '.'; i++){
                    before_decimal = before_decimal * 10.0 + (num[i] - '0');
                }
                if(i == num.size()) return before_decimal;
                _real after_decimal = 0.0;
                for(int j = num.size() - 1; j > i; j--){
                    after_decimal = (after_decimal + (num[j] - '0'))/10;
                }
                return neg ? - before_decimal - after_decimal : before_decimal + after_decimal;
            }

            class BinanceLive : public LiveBot {
            public:
                BinanceLive(std::string uri, std::string database_directory_location);
                virtual ~BinanceLive();
                virtual void start() override;
                virtual void stop() override;
                 

            private:

                class UnitTickerPayload{
                public:
                    uint64_t event_time;            // E
                    double price;                   // c
                    double high_price;              // h
                    double low_price;               // l
                    double weighed_average_price;   // w
                    std::string coin_pair;               // s

                    UnitTickerPayload(){}
                    UnitTickerPayload(const nlohmann::basic_json<> &msg){
                        event_time = msg["E"].get<uint64_t>();                          //millisec
                        price = toDouble<double> (msg["c"].get<std::string>()) ;
                        high_price = toDouble<double> (msg["h"].get<std::string>());
                        low_price = toDouble<double> (msg["l"].get<std::string>());
                        weighed_average_price = toDouble<double> (msg.find("w") != msg.end() ?  msg["w"].get<std::string>() : "-1.0");
                        coin_pair = msg["s"].get<std::string>();

                        // spdlog::debug("eventTime: {}, {} {} price: {}, high_price: {}, low_price: {}, weighted_average_price: {}, coin_pair: {}",
                        //                 event_time, EPOCH_SEC, EPOCH_MILISEC, price,high_price, low_price, weighed_average_price, coin_pair);
                    }

                
                };

                int connection_id;
                std::string uri;
                bool is_running;
                bool isStartWaiting;
                std::string database_directory_location;
                std::condition_variable stopCondition;
                std::mutex stopMutex;



                std::unordered_map< std::string, std::deque< uint64_t > > price_report_time;
                std::unordered_map< std::string, std::deque< double > > price_over_time;
                std::mutex price_array_mutex;


                std::unordered_set<std::string> fiat_coin_list;
                std::mutex fiat_coin_list_mutex;
                std::unordered_set<std::string> crypto_coin_list;
                std::mutex crypto_coin_list_mutex;
                std::unordered_set<std::string> coin_pair_list;
                std::mutex coin_pair_list_mutex;



                std::unordered_map<std::string, ds::TrieSet<double> > median_finding_ds;
                std::mutex median_finding_ds_mutex;


                std::vector<std::string> msg_buffer; 
                std::mutex msg_buffer_mutex; 

                std::atomic_bool main_scheduler_loop_running;
                uint32_t main_scheduler_loop_interval_milisec;
                std::thread main_scheduler_loop_thread;


                void processPriceTick(const std::string &msg);
                int initializeEnv();
                int setupNewCoinPair(std::string coin_pair, std::string quote, double price, double low_24h, double high_24);
                int initializeCoinPair(std::string coin_pair);
                int sync_crypto_coin_list_with_db_file();
                int sync_coin_pair_list_with_db_file();
                int newCrypto(std::string crypto);
                int newCoinPair(const std::string &coin_pair, const UnitTickerPayload &last_payload);
                std::pair<std::string,std::string> seperateCoinPair(std::string coin_pair);
                std::pair<std::string,std::string> seperateCoinPair2(std::string coin_pair);
                bool has_fiat_quote_from_list(const std::string &coin_pair);
                void main_scheduler_loop();
                void parse_sigle_ticker_array(std::string &msg, std::unordered_map<std::string, std::vector<UnitTickerPayload>> &filter_map);
                

            };

            std::shared_ptr<LiveBot> LiveBot::getInstance(const std::string &uri, const std::string &db_dir){
                return std::make_shared<BinanceLive>(uri,db_dir);
            }
            
            BinanceLive::~BinanceLive(){
                // stop();
            }

            BinanceLive::BinanceLive(std::string uri, std::string database_directory_location) : 
            uri(uri)
            , is_running(false)
            , isStartWaiting(false)
            , database_directory_location(database_directory_location)
            , main_scheduler_loop_running(false)
            , main_scheduler_loop_interval_milisec(DEFAULT_MAIN_LOOP_INTERVAL_MILISEC)
            {

                spdlog::debug("creading binance price stream bot instance");

                if(initializeEnv()){
                    spdlog::error("Initialization Failed. exiting...");
                    exit(0);
                }



            }

            int BinanceLive::initializeEnv() {
                spdlog::debug("initializing database and other stuff.");
                
                std::filesystem::path db_dir(database_directory_location);

                if(!std::filesystem::exists(db_dir)){
                    spdlog::warn("db directory does not exist. creating");
                    try {
                        std::filesystem::create_directories(db_dir);
                    }catch(std::exception &ex){
                        spdlog::error("failed to create database directory at location {} with error {}",db_dir.c_str(),ex.what());
                    }
                    if(fileutils::create_file(db_dir,FIAT_COIN_DB_FILE)){
                        spdlog::error("error while creating fiat coin database file");
                        return -1;
                    }else{
                        //start with USDT
                        std::string file_path(db_dir.c_str());
                        file_path += "/";
                        file_path += FIAT_COIN_DB_FILE;
                        std::ofstream writer(file_path, std::ios::binary | std::ios::out);
                        if(fileutils::write_string_to_file(writer,"USDT")) return -2;
                        writer.close();
                        fiat_coin_list_mutex.lock();
                        fiat_coin_list.insert("USDT");
                        fiat_coin_list_mutex.unlock();
                    }

                    if(fileutils::create_file(db_dir,CRYPTO_COIN_DB_FILE)){
                        spdlog::error("error while creating crypto coin database file");
                        return -1;
                    }

                    if(fileutils::create_file(db_dir,ALL_COIN_PAIR_DB_FILE)){
                        spdlog::error("error while creating all coin pair database file");
                        return -1;
                    }
                    return 0;
                }


                std::string fiat_coin_list_db_file_path = database_directory_location + "/" + FIAT_COIN_DB_FILE;

                if(!std::filesystem::exists(fiat_coin_list_db_file_path)){
                    spdlog::warn("Fiat coin list db file does not exist, creating...");
                    if(fileutils::create_file(db_dir,FIAT_COIN_DB_FILE)){
                        spdlog::error("error while creating fiat coin database file");
                        return -1;
                    }else{
                        //start with USDT
                        std::string file_path(db_dir.c_str());
                        file_path += "/" ;
                        file_path += FIAT_COIN_DB_FILE;
                        std::ofstream writer(file_path, std::ios::binary | std::ios::out);
                        if(fileutils::write_string_to_file(writer,"USDT")) return -2;
                        writer.close();
                        fiat_coin_list_mutex.lock();
                        fiat_coin_list.insert("USDT");
                        fiat_coin_list_mutex.unlock();
                    }
                } else {
                    
                    std::ifstream reader(fiat_coin_list_db_file_path, std::ios::binary | std::ios::in);
                    fiat_coin_list_mutex.lock();
                    while (reader.peek() != EOF)
                    {
                        fiat_coin_list.insert(fileutils::read_string_from_file(reader));
                    }
                    fiat_coin_list_mutex.unlock();
                    reader.close();
                    
                }


                std::string crypto_coin_list_db_file_path = database_directory_location + "/" + CRYPTO_COIN_DB_FILE;

                if(!std::filesystem::exists(crypto_coin_list_db_file_path)){
                    spdlog::warn("crypto coin list db file does not exist, creating...");
                    if(fileutils::create_file(db_dir,CRYPTO_COIN_DB_FILE)){
                        spdlog::error("error while creating crypto coin database file");
                        return -1;
                    }
                } else {
                    std::ifstream reader(crypto_coin_list_db_file_path, std::ios::binary | std::ios::in);
                    while (reader.peek() != EOF)
                    {
                        std::string crypto = fileutils::read_string_from_file(reader);
                        // spdlog::debug("crypto in file {}", crypto);
                        crypto_coin_list.insert(crypto);
                    }
                    reader.close();
                }

                std::string all_coin_pair_list_db_file_path = database_directory_location + "/" + ALL_COIN_PAIR_DB_FILE;

                if(!std::filesystem::exists(all_coin_pair_list_db_file_path)){
                    spdlog::warn("all coin pair list db file does not exist, creating...");
                    if(fileutils::create_file(db_dir,ALL_COIN_PAIR_DB_FILE)){
                        spdlog::error("error while creating crypto coin database file");
                        return -1;
                    }
                } else {
                    std::ifstream reader(all_coin_pair_list_db_file_path, std::ios::binary | std::ios::in);
                    coin_pair_list_mutex.lock();
                    while (reader.peek() != EOF)
                    {
                        coin_pair_list.insert(fileutils::read_string_from_file(reader));
                    }
                    coin_pair_list_mutex.unlock();
                    reader.close();
                }

                coin_pair_list_mutex.lock();
                std::vector<std::string> coin_pair_list_copy(coin_pair_list.begin(),coin_pair_list.end());
                coin_pair_list_mutex.unlock();
                for(std::string coin_pair : coin_pair_list_copy){
                    if(initializeCoinPair(coin_pair)){
                        spdlog::error("Coin pair initialization failed {}",coin_pair);
                    }
                }

                return 0;

            }

            int BinanceLive::sync_coin_pair_list_with_db_file(){
                std::string all_coin_pair_list_db_file_path = database_directory_location + "/" + ALL_COIN_PAIR_DB_FILE;
                if(!std::filesystem::exists(all_coin_pair_list_db_file_path)){
                    spdlog::warn("all coin pair list db file does not exist, creating...");
                    std::filesystem::path db_dir(database_directory_location);
                    if(fileutils::create_file(db_dir,ALL_COIN_PAIR_DB_FILE)){
                        spdlog::error("error while creating crypto coin database file");
                        return -1;
                    }
                } 
                std::ofstream writter(all_coin_pair_list_db_file_path, std::ios::binary | std::ios::out);
                coin_pair_list_mutex.lock();
                for(std::string coin_pair : coin_pair_list){
                    fileutils::write_string_to_file(writter,coin_pair);
                }
                coin_pair_list_mutex.unlock();
                writter.close();
                return 0;
            }
            int BinanceLive::sync_crypto_coin_list_with_db_file(){
                std::string crypto_coin_list_db_file_path = database_directory_location + "/" + CRYPTO_COIN_DB_FILE;
                if(!std::filesystem::exists(crypto_coin_list_db_file_path)){
                    spdlog::warn("all coin pair list db file does not exist, creating...");
                    std::filesystem::path db_dir(database_directory_location);
                    if(fileutils::create_file(db_dir,CRYPTO_COIN_DB_FILE)){
                        spdlog::error("error while creating crypto coin database file");
                        return -1;
                    }
                } 
                std::ofstream writter(crypto_coin_list_db_file_path, std::ios::binary | std::ios::out);
                crypto_coin_list_mutex.lock();
                for(std::string crypto : crypto_coin_list){
                    // spdlog::debug("writting {} to {}", crypto, crypto_coin_list_db_file_path);
                    fileutils::write_string_to_file(writter,crypto);
                }
                crypto_coin_list_mutex.unlock();
                writter.close();
                return 0;
            }



            std::pair<std::string,std::string> BinanceLive::seperateCoinPair(std::string coin_pair){
                std::string base, quote;
                crypto_coin_list_mutex.lock();
                std::vector<std::string> crypto_coin_list_cpy(crypto_coin_list.begin(),crypto_coin_list.end());
                fiat_coin_list_mutex.lock();
                //find through base coin
                std::string empty_str = "";
                auto ret = std::make_pair(empty_str,empty_str);
                for(std::string can: crypto_coin_list_cpy){
                    if(coin_pair.size() > can.size() && coin_pair.substr(0,can.size()) == can){
                        base = can;
                        quote = coin_pair.substr(can.size());
                        if(fiat_coin_list.find(quote) != fiat_coin_list.end()){
                            ret =  std::make_pair(base,quote);
                            break;
                        } else if (crypto_coin_list.find(quote) != crypto_coin_list.end()){
                            //crypto pair
                            ret =  std::make_pair(base,quote);
                            break;
                        }
                    }
                }
                fiat_coin_list_mutex.unlock();
                crypto_coin_list_mutex.unlock();

                //find through quote coin

                //report new coin pair 

                return ret;

            }
            std::pair<std::string,std::string> BinanceLive::seperateCoinPair2(std::string coin_pair){
                std::string base, quote;
                fiat_coin_list_mutex.lock();
                std::vector<std::string> fiat_coin_list_cpy(fiat_coin_list.begin(),fiat_coin_list.end());
                fiat_coin_list_mutex.unlock();
                //find through base coin
                std::string empty_str = "";
                auto ret = std::make_pair(empty_str,empty_str);
                for(std::string can: fiat_coin_list_cpy){
                    // spdlog::debug("seperateCoinPair2 coin pair {} candidate {}",coin_pair, can);
                    if(coin_pair.size() > can.size() && coin_pair.substr(coin_pair.size()-can.size()) == can){
                        quote = can;
                        base = coin_pair.substr(0,coin_pair.size()-can.size());
                        // spdlog::debug("seperateCoinPair2 coin pair {} candidate {} base {} quote {}",coin_pair, can, base, quote);
                        // if(crypto_coin_list.find(base) != crypto_coin_list.end()){
                        ret =  std::make_pair(base,quote);
                        break;
                        // }
                    }
                }
                // spdlog::debug("seperateCoinPair2 coin pair {} ret {} {}",coin_pair, ret.first, ret.second);

                return ret;

            }

            int BinanceLive::initializeCoinPair(std::string coin_pair){
                // db file should exist, if not, we should delete it's existance from database;

                std::string db_file_name = "_" + coin_pair + COIN_PRICE_DB_FILE_SUFFIX;
                std::string db_file_path = database_directory_location + "/" + db_file_name;
                if(!std::filesystem::exists(db_file_path)){
                    spdlog::error("coin pair database file does not exist, removing coin");
                    coin_pair_list_mutex.lock();
                    coin_pair_list.erase(coin_pair);
                    coin_pair_list_mutex.unlock();
                    sync_coin_pair_list_with_db_file();
                    return -1;
                }

                //base quote

                std::pair<std::string, std::string> tmp = seperateCoinPair(coin_pair);

                std::string base  = tmp.first;
                std::string quote  = tmp.second;

                if( base.empty() or quote.empty()){
                    spdlog::error("could not seperate coin pair from {}", coin_pair);
                    return -1;
                }

                //read_database
                //write coin_min, coin_max, coin_price_array

                std::ifstream reader(db_file_path,std::ios::binary | std::ios::in);

                price_array_mutex.lock();

                std::deque<uint64_t> &report_time_array_ref = price_report_time[coin_pair];
                std::deque<double> &price_array_ref = price_over_time[coin_pair];

                median_finding_ds_mutex.lock();

                ds::TrieSet<double> &median_finding_ds_ref = median_finding_ds[coin_pair];

                while(reader.peek() != EOF){
                    uint64_t report_time = fileutils::read_uint64(reader);
                    if(reader.peek() == EOF){
                        spdlog::warn("inconsitent database file for coint pair {}", coin_pair);
                        break;
                    }
                    double price = fileutils::read_double(reader);

                    report_time_array_ref.push_back(report_time);
                    price_array_ref.push_back(price);
                    median_finding_ds_ref.insert(price);

                }
                reader.close();
                median_finding_ds_mutex.unlock();
                price_array_mutex.unlock();

                return 0;
            }


            void BinanceLive::start(){

                if(is_running){
                    spdlog::error("Live bot alraedy running");
                    return;
                }
                is_running = true;

                if(!main_scheduler_loop_running){
                    main_scheduler_loop_thread = std::thread(&BinanceLive::main_scheduler_loop,this);
                    main_scheduler_loop_running = true;

#if IS_WINDOWS
                    if(SetThreadPriority(main_scheduler_loop_thread.native_handle(), THREAD_PRIORITY_HIGHEST)) {
                        spdlog::error("Failed to set priority for main_scheduler_loop thread, Thread Priority set Error");
                    }
#else

                    struct sched_param schdparam;
                    int threadpolicy = 0;
                    pthread_getschedparam(main_scheduler_loop_thread.native_handle(), &threadpolicy, &schdparam);
                    schdparam.sched_priority = sched_get_priority_max(threadpolicy);
                    if(pthread_setschedparam(main_scheduler_loop_thread.native_handle(), threadpolicy, &schdparam) != 0) {
                        spdlog::error("Failed to set priority for main_scheduler_loop thread, Thread Priority set Error");
                    }
#endif
                }

                // {
                //     std::unique_lock<std::mutex> l(stopMutex);
                //     isStartWaiting = true;
                //     stopCondition.wait(l);
                //     isStartWaiting = false;
                // }
                
                connection_id = bot::stream::websocket::websocket_endpoint::get_instance().connect(uri,[&](const std::string &msg){
                    msg_buffer_mutex.lock();
                    msg_buffer.push_back(msg);
                    msg_buffer_mutex.unlock();
                });

            }

            void BinanceLive::stop() {

                spdlog::info("Stop called! lossing connection and clearing memory");
			    websocketpp::lib::error_code ec;
                
                bot::stream::websocket::websocket_endpoint::get_instance().close(connection_id, websocketpp::close::status::going_away, "will come back later" ,ec);

                main_scheduler_loop_running = false;
                if(main_scheduler_loop_thread.joinable()) {
                    main_scheduler_loop_thread.join();
                } else {
                    spdlog::error("cant join main_scheduler_loop_thread, not joinable");
                }
                //process buffer
                //synk with ds
                for(auto it : median_finding_ds){
                    it.second.clear();
                }
                is_running = false;
            }

            void BinanceLive::parse_sigle_ticker_array(std::string &msg, std::unordered_map<std::string, std::vector<UnitTickerPayload>> &filter_map){
                auto msg_json = nlohmann::json::parse(msg);

                for(auto &single_tikcer_json : msg_json){


                    std::string coin_pair = single_tikcer_json["s"].get<std::string>();
                    coin_pair_list_mutex.lock();
                    auto it = coin_pair_list.find(coin_pair);
                    bool pair_exist = (it != coin_pair_list.end());
                    coin_pair_list_mutex.unlock();

                    // spdlog::debug("coin pair {} pair exist {}", coin_pair, pair_exist);


                    if(not pair_exist && has_fiat_quote_from_list(coin_pair)){
                        std::pair<std::string, std::string> sep = seperateCoinPair2(coin_pair);

                        std::string crypto = sep.first;

                        // spdlog::debug("seperated from pair {} {} {}",coin_pair, sep.first,sep.second);
                        crypto_coin_list_mutex.lock();
                        auto it = crypto_coin_list.find(crypto);
                        bool crypto_exist = (it != crypto_coin_list.end());
                        crypto_coin_list_mutex.unlock();


                        if(!crypto_exist){
                            //new crypto !!
                            newCrypto(crypto);
                        }
                        //new pair 
                        UnitTickerPayload t(single_tikcer_json);
                        newCoinPair(coin_pair,t);
                        filter_map[t.coin_pair].push_back(t);

                    }else if(pair_exist){
                        UnitTickerPayload t(single_tikcer_json);
                        filter_map[t.coin_pair].push_back(t);
                    }


                }
            }


            void BinanceLive::main_scheduler_loop(){
                spdlog::info("main scheduler loop started, running interval {} milisec", main_scheduler_loop_interval_milisec);

                while (main_scheduler_loop_running)
                {
                    /* code */
                    msg_buffer_mutex.lock();
                    std::vector<std::string> msg_buffer_cpy(msg_buffer.begin(),msg_buffer.end());
                    msg_buffer.clear();
                    msg_buffer_mutex.unlock();

                    spdlog::debug("processing {} data.", msg_buffer_cpy.size());

                    std::unordered_map<std::string, std::vector<UnitTickerPayload>> filtered_ticker;

                    for(std::string &msg : msg_buffer_cpy){
                        parse_sigle_ticker_array(msg,filtered_ticker);
                    }

                    
                    spdlog::debug("coin picked {}", filtered_ticker.size());


                    timespec sleep_time = { main_scheduler_loop_interval_milisec/1000, main_scheduler_loop_interval_milisec*1000 - (main_scheduler_loop_interval_milisec/1000)*1000000};
                    timespec reamining_sleep_time;
                    nanosleep(&sleep_time,&reamining_sleep_time);

                }
                

                spdlog::info("stopping main scheduler loop");
            }

            bool BinanceLive::has_fiat_quote_from_list(const std::string &coin_pair){
                fiat_coin_list_mutex.lock();
                bool ret = false;
                for(auto can : fiat_coin_list){
                    if(coin_pair.size() > can.size() and coin_pair.substr(can.size() -1) == can){
                        ret =  true;
                        break;
                    }
                }
                fiat_coin_list_mutex.unlock();
                return ret;
            }

            int BinanceLive::newCrypto(std::string crypto){
                crypto_coin_list_mutex.lock();
                crypto_coin_list.insert(crypto);
                crypto_coin_list_mutex.unlock();
                return sync_crypto_coin_list_with_db_file();

            }
            int BinanceLive::newCoinPair(const std::string &coin_pair,const UnitTickerPayload &last_payload){
                coin_pair_list_mutex.lock();
                coin_pair_list.insert(coin_pair);
                coin_pair_list_mutex.unlock();
                sync_coin_pair_list_with_db_file();

                //indialize database file 
                std::string db_file_name = "_" + coin_pair + COIN_PRICE_DB_FILE_SUFFIX;
                std::string db_file_path = database_directory_location + "/" + db_file_name;
                //create db file
                if(fileutils::create_file(database_directory_location,db_file_name)){
                    spdlog::error("Fail to create database file for coinpair {}",coin_pair);
                    return -1;
                }
 
                std::ofstream writer(db_file_path,std::ios::binary | std::ios::out);
                fileutils::write_uint64(writer,last_payload.event_time);
                fileutils::write_double(writer,last_payload.high_price);
                fileutils::write_uint64(writer,last_payload.event_time);
                fileutils::write_double(writer,last_payload.low_price);
                writer.close();

                price_array_mutex.lock();
                price_report_time[coin_pair].push_back(last_payload.event_time);
                price_over_time[coin_pair].push_back(last_payload.high_price);

                price_report_time[coin_pair].push_back(last_payload.event_time);
                price_over_time[coin_pair].push_back(last_payload.low_price);
                price_array_mutex.unlock();

                median_finding_ds_mutex.lock();
                median_finding_ds[coin_pair].insert(last_payload.high_price);
                median_finding_ds[coin_pair].insert(last_payload.low_price);
                median_finding_ds_mutex.unlock();
                return 0;
            }

            // void BinanceLive::processPriceTick(const std::string &msg) {
            //     auto j = nlohmann::json::parse(msg);
            //     // int total_count = 0;
            //     // int usdt_total_count = 0;
            //     // int busd_total_count = 0;
            //     for(auto item : j){


            //         spdlog::debug(item.dump());
            //         UnitTickerPayload t(item);
            //         break;
            //         std::string coin_pair = item["s"].get<std::string>();
            //         coin_pair_list_mutex.lock();
            //         auto it = coin_pair_list.find(coin_pair);
            //         bool pair_exist = (it != coin_pair_list.end());
            //         coin_pair_list_mutex.unlock();

            //         if(not pair_exist && has_fiat_quote_from_list(coin_pair)){
            //             //new coin_pair
            //             //setup
            //         }else if(pair_exist){
            //             //parse,lock,push,release

            //         }
            //         // total_count++;
            //     }
            //     // std::cout<<total_count<<"\n";
            // }


            



            
        } // namespace live
        
        
    } // namespace stream


    
    
} // namespace bot
