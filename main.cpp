#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <deque>
#include <random>



template<typename _real> _real toDouble(std::string num){
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
    return before_decimal + after_decimal;
}


// std::string  to_string(const char * p){

// }


int main(int argc, char ** argv){
    using namespace std;
    // std::mt19937 r(time(0));

    // cout.precision(12);cout<<fixed;

    // cout<<toDouble<double>("205.09")<<endl;


    string x = "love";

    string y = "kola" + x + " peta";

    cout<<y<<endl;

    return 0;
}



// // #include <iostream>
// // #include <cpprest/http_client.h>
// // #include <cpprest/filestream.h>
// // #include <cpprest/uri.h>
// // #include <cpprest/json.h>
 
// // using namespace utility;
// // using namespace web;
// // using namespace web::http;
// // using namespace web::http::client;
// // using namespace concurrency::streams;
 
// // int main() {
// // 	// Create a file stream to write the received file into it.

// // 	std::cout<<"Hello"<<std::endl;
// // 	return 0;
// // }

// // #include <websocketpp/config/asio_no_tls_client.hpp>
// #include <websocketpp/config/asio_client.hpp>
// #include <websocketpp/client.hpp>

// #include <iostream>
// #include <string>
 

// typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
// typedef std::shared_ptr<boost::asio::ssl::context> context_ptr;

// class connection_metadata {
// public:
//     typedef std::shared_ptr<connection_metadata> ptr;
 
//     connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri)
//       : m_id(id)
//       , m_hdl(hdl)
//       , m_status("Connecting")
//       , m_uri(uri)
//       , m_server("N/A")
//     {}
 
//     void on_open(client * c, websocketpp::connection_hdl hdl) {
//         m_status = "Open";
 
//         client::connection_ptr con = c->get_con_from_hdl(hdl);
//         m_server = con->get_response_header("Server");
//     }
 
//     void on_fail(client * c, websocketpp::connection_hdl hdl) {
//         m_status = "Failed";
 
//         client::connection_ptr con = c->get_con_from_hdl(hdl);
//         m_server = con->get_response_header("Server");
//         m_error_reason = con->get_ec().message();
//     }

// 	void on_close(client * c, websocketpp::connection_hdl hdl) {
// 		m_status = "Closed";
// 		client::connection_ptr con = c->get_con_from_hdl(hdl);
// 		std::stringstream s;
// 		s << "close code: " << con->get_remote_close_code() << " (" 
// 		<< websocketpp::close::status::get_string(con->get_remote_close_code()) 
// 		<< "), close reason: " << con->get_remote_close_reason();
// 		m_error_reason = s.str();
// 	}

// 	void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) {
// 		std::cout<<"Recieved: "<<msg->get_payload()<<std::endl;
// 		if (msg->get_opcode() == websocketpp::frame::opcode::text) {
// 			m_messages.push_back(msg->get_payload());
// 		} else {
// 			m_messages.push_back(websocketpp::utility::to_hex(msg->get_payload()));
// 		}
// 	}

//     context_ptr on_tls_init() {
//     // establishes a SSL connection
//         context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);

//         try {
//             ctx->set_options(boost::asio::ssl::context::default_workarounds |
//                             boost::asio::ssl::context::no_sslv2 |
//                             boost::asio::ssl::context::no_sslv3 |
//                             boost::asio::ssl::context::single_dh_use);
//         } catch (std::exception &e) {
//             std::cout << "Error in context pointer: " << e.what() << std::endl;
//         }
//         return ctx;
//     }

// 	std::string get_status() const { return m_status; }

// 	websocketpp::connection_hdl get_hdl() const { return m_hdl; }

// 	int get_id() const { return m_id; }
// 	void record_sent_message(std::string message) {
// 		m_messages.push_back(">> " + message);
// 	}
 
//     friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);
// private:
//     int m_id;
//     websocketpp::connection_hdl m_hdl;
//     std::string m_status;
//     std::string m_uri;
//     std::string m_server;
//     std::string m_error_reason;
// 	std::vector<std::string> m_messages;
// };
 
// std::ostream & operator<< (std::ostream & out, connection_metadata const & data) {
//     out << "> URI: " << data.m_uri << "\n"
//         << "> Status: " << data.m_status << "\n"
//         << "> Remote Server: " << (data.m_server.empty() ? "None Specified" : data.m_server) << "\n"
//         << "> Error/close reason: " << (data.m_error_reason.empty() ? "N/A" : data.m_error_reason)<<"\n"
// 		<< "> Messages Processed: (" << data.m_messages.size() << ") \n";
 
// 	std::vector<std::string>::const_iterator it;
// 	for (it = data.m_messages.begin(); it != data.m_messages.end(); ++it) {
// 		out << *it << "\n";
// 	}
 
//     return out;
// }


// class websocket_endpoint {
// public:
//     websocket_endpoint () : m_next_id(0) {
//         m_endpoint.clear_access_channels(websocketpp::log::alevel::all);
//         m_endpoint.clear_error_channels(websocketpp::log::elevel::all);
 
//         m_endpoint.init_asio();
//         m_endpoint.set_tls_init_handler([](websocketpp::connection_hdl hdl){
//             context_ptr ctx = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
//             try {
//                 ctx->set_options(boost::asio::ssl::context::default_workarounds |
//                                 boost::asio::ssl::context::no_sslv2 |
//                                 boost::asio::ssl::context::no_sslv3 |
//                                 boost::asio::ssl::context::single_dh_use);
//             } catch (std::exception &e) {
//                 std::cout << "Error in context pointer: " << e.what() << std::endl;
//             }
//             return ctx;
//         });
//         m_endpoint.start_perpetual();
 
//         m_thread.reset(new websocketpp::lib::thread(&client::run, &m_endpoint));
//     }
// 	~websocket_endpoint() {
// 		m_endpoint.stop_perpetual();
		
// 		for (con_list::const_iterator it = m_connection_list.begin(); it != m_connection_list.end(); ++it) {
// 			if (it->second->get_status() != "Open") {
// 				// Only close open connections
// 				continue;
// 			}
			
// 			std::cout << "> Closing connection " << it->second->get_id() << std::endl;
			
// 			websocketpp::lib::error_code ec;
// 			m_endpoint.close(it->second->get_hdl(), websocketpp::close::status::going_away, "", ec);
// 			if (ec) {
// 				std::cout << "> Error closing connection " << it->second->get_id() << ": "  
// 						<< ec.message() << std::endl;
// 			}
// 		}
		
// 		m_thread->join();
// 	}
 
//     int connect(std::string const & uri) {
//         websocketpp::lib::error_code ec;
 
//         client::connection_ptr con = m_endpoint.get_connection(uri, ec);
 
//         if (ec) {
//             std::cout << "> Connect initialization error: " << ec.message() << std::endl;
//             return -1;
//         }
 
//         int new_id = m_next_id++;
//         connection_metadata::ptr metadata_ptr(new connection_metadata(new_id, con->get_handle(), uri));
//         m_connection_list[new_id] = metadata_ptr;
 
//         con->set_open_handler(websocketpp::lib::bind(
//             &connection_metadata::on_open,
//             metadata_ptr,
//             &m_endpoint,
//             websocketpp::lib::placeholders::_1
//         ));
//         con->set_fail_handler(websocketpp::lib::bind(
//             &connection_metadata::on_fail,
//             metadata_ptr,
//             &m_endpoint,
//             websocketpp::lib::placeholders::_1
//         ));

// 		con->set_message_handler(websocketpp::lib::bind(
// 			&connection_metadata::on_message,
// 			metadata_ptr,
// 			websocketpp::lib::placeholders::_1,
// 			websocketpp::lib::placeholders::_2
// 		));

//         con->set_ping_handler([&](websocketpp::connection_hdl hdl,std::string mesage){
//             // m_endpoint.send(hdl,"Ping Recieved",websocketpp::frame::opcode::PONG);
//             return true;
//         });
 
//         m_endpoint.connect(con);
 
//         return new_id;
//     }

// 	void close(int id, websocketpp::close::status::value code, std::string reason) {
// 		websocketpp::lib::error_code ec;
		
// 		con_list::iterator metadata_it = m_connection_list.find(id);
// 		if (metadata_it == m_connection_list.end()) {
// 			std::cout << "> No connection found with id " << id << std::endl;
// 			return;
// 		}
// 		m_endpoint.close(metadata_it->second->get_hdl(), code, reason, ec);
// 		if (ec) {
// 			std::cout << "> Error initiating close: " << ec.message() << std::endl;
// 		}	
// 	}

// 	void send(int id, std::string message) {
//     	websocketpp::lib::error_code ec;
// 		con_list::iterator metadata_it = m_connection_list.find(id);
// 		if (metadata_it == m_connection_list.end()) {
// 			std::cout << "> No connection found with id " << id << std::endl;
// 			return;
// 		}
		
// 		m_endpoint.send(metadata_it->second->get_hdl(), message, websocketpp::frame::opcode::text, ec);

// 		if (ec) {
// 			std::cout << "> Error sending message: " << ec.message() << std::endl;
// 			return;
// 		}		
// 		metadata_it->second->record_sent_message(message);
// 	}
 
//     connection_metadata::ptr get_metadata(int id) const {
//         con_list::const_iterator metadata_it = m_connection_list.find(id);
//         if (metadata_it == m_connection_list.end()) {
//             return connection_metadata::ptr();
//         } else {
//             return metadata_it->second;
//         }
//     }
// private:
//     typedef std::map<int,connection_metadata::ptr> con_list;
 
//     client m_endpoint;
//     websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;
 
//     con_list m_connection_list;
//     int m_next_id;
// };
 
// // int main() {
// //     bool done = false;
// //     std::string input;
// //     websocket_endpoint endpoint;
 
// //     while (!done) {
// //         std::cout << "Enter Command: ";
// //         std::getline(std::cin, input);
 
// //         if (input == "q") {
// //             done = true;
// //         } else if (input == "help") {
// //             std::cout
// //                 << "\nCommand List:\n"
// //                 << "connect <ws uri>\n"
// //                 << "show <connection id>\n"
// //                 << "help: Display this help text\n"
// //                 << "q: Exit the program\n"
// //                 << std::endl;
// //         } else if (input.substr(0,7) == "connect") {
// //             int id = endpoint.connect("ws://localhost:9898/");
// //             if (id != -1) {
// //                 std::cout << "> Created connection with id " << id << std::endl;
// //             }
// //         } else if (input.substr(0,4) == "show") {
// //             int id = atoi(input.substr(5).c_str());
 
// //             connection_metadata::ptr metadata = endpoint.get_metadata(id);
// //             if (metadata) {
// //                 std::cout << *metadata << std::endl;
// //             } else {
// //                 std::cout << "> Unknown connection id " << id << std::endl;
// //             }
// //         } else if (input.substr(0,5) == "close") {
// // 			std::stringstream ss(input);
			
// // 			std::string cmd;
// // 			int id;
// // 			int close_code = websocketpp::close::status::normal;
// // 			std::string reason;
			
// // 			ss >> cmd >> id >> close_code;
// // 			std::getline(ss,reason);
			
// // 			endpoint.close(id, close_code, reason);
// // 		} else if (input.substr(0,4) == "send") {
// // 			std::stringstream ss(input);
				
// // 				std::string cmd;
// // 				int id;
// // 				std::string message = "";
				
// // 				ss >> cmd >> id;
// // 				std::getline(ss,message);
				
// // 				endpoint.send(id, message);
// // 		} 
// // 		else {
// //             std::cout << "> Unrecognized Command" << std::endl;
// //         }
// //     }
 
// // }


// int main(int argc, char ** argv){

//     bool done = false;
//     std::string input;
//     websocket_endpoint endpoint;
// 	int id = endpoint.connect("wss://testnet.binance.vision/ws/!ticker@arr"); //Individual Symbol Mini Ticker Stream
// 	// int id = endpoint.connect("wss://testnet.binance.vision/ws/ETHUSDT@miniTicker"); //Individual Symbol Mini Ticker Stream
// 	// int id = endpoint.connect("wss://stream.binance.com:9443/ws/ETHUSDT@miniTicker"); //Individual Symbol Mini Ticker Stream

// 	if(id != 0){
// 		std::cout<<"connection failed"<<std::endl;
// 		return 0;
// 	}

//     // std::string sub_str = "{ \"method\": \"SUBSCRIBE\", \"params\": [\"ethudst@depth\"], \"id\": 1}";
//     // endpoint.send(id,sub_str);
//     // while(endpoint.get_metadata(id)->get_status() != "Open" ){
//     //     sleep(1);
//     // }

//     // endpoint.send(id,sub_str);


// 	while (!done) {
// 		std::cout << "Enter Command: ";
//         std::getline(std::cin, input);
 
//         if (input == "q") {
//             done = true;
//         } else if(input == "s") {
//             connection_metadata::ptr metadata = endpoint.get_metadata(id);
//             if (metadata) {
//                 std::cout << *metadata << std::endl;
//             } else {
//                 std::cout << "> Unknown connection id " << id << std::endl;
//             }
// 		}
// 	}

// 	return 0;
// }


// /*
// {
//   "method": "SUBSCRIBE",
//   "params": [
//     "btcusdt@aggTrade",
//     "btcusdt@depth"
//   ],
//   "id": 1
// }
// */

// //ping handler: https://stackoverflow.com/questions/25221566/how-to-implement-websocket-ping-handler