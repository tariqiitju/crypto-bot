#ifndef _WEBSOCKET_ENDPOINT_H
#define _WEBSOCKET_ENDPOINT_H
//TODO : Ping handler

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include "spdlog/spdlog.h"

namespace bot
{
    namespace stream
    {
        namespace websocket
        {
            typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
            typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;
            typedef std::function <void(const std::string& status)> simple_callback;

            class connection_metadata {
            public:
                typedef std::shared_ptr<connection_metadata> ptr;
            
                connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri)
                : m_id(id)
                , m_hdl(hdl)
                , m_status("Connecting")
                , m_uri(uri)
                , m_server("N/A")
                , m_incoming_message_count(0)
                , m_outgoing_message_count(0)
                , m_message_handler(nullptr)
                {}
            
                void on_open(client * c, websocketpp::connection_hdl hdl) {
                    spdlog::debug("open handler called");
                    m_status = "Open";

            
                    client::connection_ptr con = c->get_con_from_hdl(hdl);
                    m_server = con->get_response_header("Server");
                }
            
                void on_fail(client * c, websocketpp::connection_hdl hdl) {
                    m_status = "Failed";
            
                    client::connection_ptr con = c->get_con_from_hdl(hdl);
                    m_server = con->get_response_header("Server");
                    m_error_reason = con->get_ec().message();
                }

                void on_close(client * c,websocketpp::connection_hdl hdl) {
                    spdlog::debug("on close called");
                    m_status = "Closed";
                    client::connection_ptr con = c->get_con_from_hdl(hdl);
                    std::stringstream s;
                    s << "close code: " << con->get_remote_close_code() << " (" 
                    << websocketpp::close::status::get_string(con->get_remote_close_code()) 
                    << "), close reason: " << con->get_remote_close_reason();
                    m_error_reason = s.str();
                }

                void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
                    // spdlog::debug("Recieved: {}",msg->get_payload().c_str());
                    // if (msg->get_opcode() == websocketpp::frame::opcode::text) {
                    //     m_messages.push_back(msg->get_payload());
                    // } else {
                    //     m_messages.push_back(websocketpp::utility::to_hex(msg->get_payload()));
                    // }
                    if(nullptr != m_message_handler)
                        m_message_handler(msg->get_payload());

                    m_incoming_message_count++;
                }

                context_ptr on_tls_init() {
                // establishes a SSL connection
                    context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

                    try {
                        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                                        boost::asio::ssl::context::no_sslv2 |
                                        boost::asio::ssl::context::no_sslv3 |
                                        boost::asio::ssl::context::single_dh_use);
                    } catch (std::exception &e) {
                        spdlog::error("Error in context pointer: {}",e.what());
                    }
                    return ctx;
                }

                std::string get_status() const { return m_status; }

                websocketpp::connection_hdl get_hdl() const { return m_hdl; }

                int get_id() const { return m_id; }
                void set_message_handler(const simple_callback &message_handler){ m_message_handler = message_handler;}
                void count_sent_message(std::string message) {
                    // m_messages.push_back(">> " + message);
                    m_outgoing_message_count++;
                }
            
                friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data) {
                    out << "> URI: " << data.m_uri << "\n"
                        << "> Status: " << data.m_status << "\n"
                        << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
                        << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason)<<"\n"
                        << "> Tola Messages Processed: (" << (data.m_incoming_message_count + data.m_outgoing_message_count) << ") \n"
                        << "> Tola Incoming Messages: (" << data.m_incoming_message_count << ") \n"
                        << "> Tola Outgoing Messages : (" << data.m_outgoing_message_count << ") \n";            
                    return out;
                }
            private:
                int m_id;
                websocketpp::connection_hdl m_hdl;
                std::string m_status;
                std::string m_uri;
                std::string m_server;
                std::string m_error_reason;
                // std::vector<std::string> m_messages;
                int m_incoming_message_count;
                int m_outgoing_message_count;
                simple_callback m_message_handler;
            };
            
            // std::ostream & operator<< (std::ostream & out, connection_metadata const & data)

            class websocket_endpoint {
            public:
                websocket_endpoint () : m_next_id(0) {
                    m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
                    m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
            
                    m_endpoint.init_asio();
                    m_endpoint.set_tls_init_handler([](websocketpp::connection_hdl hdl){
                        context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
                        try {
                            ctx->set_options(boost::asio::ssl::context::default_workarounds |
                                            boost::asio::ssl::context::no_sslv2 |
                                            boost::asio::ssl::context::no_sslv3 |
                                            boost::asio::ssl::context::single_dh_use);
                        } catch (std::exception &e) {
                            spdlog::error("Error in context pointer: {}",e.what());
                        }
                        return ctx;
                    });
                    m_endpoint.start_perpetual();
            
                    m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
                }
                ~websocket_endpoint() {
                    m_endpoint.stop_perpetual();
                    
                    for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
                        if (it->second->get_status() != "Open") {
                            // Only close open connections
                            continue;
                        }
                        
                        spdlog::info("Closing connection {}", it->second->get_id());
                        websocketpp::lib::error_code ec;
                        spdlog::debug("connection status 1 {}", it->second->get_status()); 
                        m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
                        spdlog::debug("connection status 2 {}", it->second->get_status()); 

                        if (ec) {
                            spdlog::error("> Error closing connection {}: {}", it->second->get_id(), ec.message().c_str());
                        }
                    }
                    
                    m_thread->join();
                }
                int connect(std::string const & uri, simple_callback message_handler) {
                    spdlog::debug("connecting to {}",uri);
                    websocketpp::lib::error_code ec;
                    client::connection_ptr con = m_endpoint.get_connection(uri, ec);
                    if (ec) {
                        spdlog::error("> Connect initialization error: {}", ec.message().c_str());
                        return -1;
                    }
            
                    int new_id = m_next_id++;
                    connection_metadata::ptr metadata_ptr(new connection_metadata(new_id, con->get_handle(), uri));
                    metadata_ptr->set_message_handler(message_handler);
                    m_connection_list[new_id] = metadata_ptr;
            
                    con->set_open_handler(websocketpp::lib::bind(
                        &connection_metadata::on_open,
                        metadata_ptr,
                        &m_endpoint,
                        websocketpp::lib::placeholders::_1
                    ));
                    con->set_fail_handler(websocketpp::lib::bind(
                        &connection_metadata::on_fail,
                        metadata_ptr,
                        &m_endpoint,
                        websocketpp::lib::placeholders::_1
                    ));

                    con->set_message_handler(websocketpp::lib::bind(
                        &connection_metadata::on_message,
                        metadata_ptr,
                        websocketpp::lib::placeholders::_1,
                        websocketpp::lib::placeholders::_2
                    ));

                    con->set_close_handler( websocketpp::lib::bind(
                        &connection_metadata::on_close,
                        metadata_ptr,
                        &m_endpoint,
                        websocketpp::lib::placeholders::_1
                    ));

                    // con->set_close_handler([](websocketpp::connection_hdl hdl){
                    //     spdlog::debug("close handler called there");
                    // });

                    con->set_ping_handler([&](websocketpp::connection_hdl hdl,std::string mesage){
                        // m_endpoint.send(hdl,"Ping Recieved",websocketpp::frame::opcode::PONG);
                        return true;
                    });
            
                    m_endpoint.connect(con);
                    spdlog::debug("connection request to {} finished processing",uri);
                    return new_id;
                }

            
                int connect(std::string const & uri) {
                    return connect(uri, nullptr);
                }

                void close(int id, websocketpp::close::status::value code,const std::string &reason, websocketpp::lib::error_code &ec) {
                    
                    con_list::iterator metadata_it = m_connection_list.find(id);
                    if (metadata_it == m_connection_list.end()) {
                        spdlog::info("> No connection found with id {}",id);
                        return;
                    }
                    spdlog::debug("connection status 3 {}", metadata_it->second->get_status()); 
                    m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
                    spdlog::debug("connection status 4 {}", metadata_it->second->get_status()); 
                    if (ec) {
                        spdlog::info("> Error initiating close: {}",ec.message());
                    }	
                }

                void send(int id, std::string message) {
                    websocketpp::lib::error_code ec;
                    con_list::iterator metadata_it = m_connection_list.find(id);
                    if (metadata_it == m_connection_list.end()) {
                        spdlog::info("> No connection found with id {}",id);
                        return;
                    }
                    
                    m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);

                    if (ec) {
                        spdlog::info("> Error sending message: {}",ec.message().c_str());
                        return;
                    }		
                    metadata_it->second->count_sent_message(message);
                }
            
                connection_metadata::ptr get_metadata(int id) const {
                    con_list::const_iterator metadata_it = m_connection_list.find(id);
                    if (metadata_it == m_connection_list.end()) {
                        return connection_metadata::ptr();
                    } else {
                        return metadata_it->second;
                    }
                }
                static websocket_endpoint& get_instance(){
                    // https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
                    static websocket_endpoint instance;
                    return instance;
                }
                websocket_endpoint(websocket_endpoint const&) = delete;
                void operator = (websocket_endpoint const&) = delete;
            private:
                typedef std::map<int,connection_metadata::ptr> con_list;
            
                client m_endpoint;
                websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
            
                con_list m_connection_list;
                int m_next_id;
                //c++03
                // websocket_endpoint(websocket_endpoint const&);
                // void operator=(websocket_endpoint const&);
            };
            
        } // namespace websocket
        
        
    } // namespace stream
    
    
} // namespace bot
#endif