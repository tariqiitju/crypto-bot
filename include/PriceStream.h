#ifndef _PRICE_STREAM_H
#define _PRICE_STREAM_H

#include <string>
#include <memory>
namespace bot
{
    namespace stream
    {
        namespace live
        {

            class LiveBot{
            public:
                // static void printHello();
                static std::shared_ptr<LiveBot> getInstance(const std::string &uri, const std::string &db_dir);
                virtual ~LiveBot() {}
                virtual void start() = 0;
                virtual void stop() = 0;


            };

        } // namespace live
        
        
    } // namespace stream
    
    
} // namespace bot

#endif