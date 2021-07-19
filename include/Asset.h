#ifndef _ASSET_H
#define _ASSET_H
#include <string>
namespace bot
{
    namespace asset
    {
        class SpotTradeUnit
        {
        private:
            std::string coin_pair;
            std::string base;
            std::string quote;
            double price;
            double low_24h;
            double high_24h;
            double median;
            uint32_t median_interval_in_hour;
            uint64_t time;
            /* data */
        public:
            SpotTradeUnit(/* args */);
            virtual ~SpotTradeUnit(){}
        };
        
        SpotTradeUnit::SpotTradeUnit(/* args */)
        {
        }

                
    } // namespace asset
    
    
} // namespace bot



#endif
