#ifndef RTT_CLIENT
#define RTT_CLIENT

#include <asio.hpp>
#include <print>
#include <thread>
#include <vector>

#include "csv.hpp"


using std::string;
using std::jthread;
using asio::ip::tcp;
using std::println;
using std::vector;
using std::error_code;

namespace rtt {

    class client {
    public:
        client(string ip, string port) : _socket(_io), _resolver(_io) {
            _io_thred = std::thread([this] {
                println("start asio::_io");
                _io.run();
            }); 

            // _socket = tcp::socket(_io);
            // _resolver = tcp::resolver(_io);

            data_exchange.rx_buffer.resize(4*1024);
            data_exchange.tx_buffer.resize(4*1024);

            asio::async_connect( _socket, _resolver.resolve(ip, port),
            [this](error_code ec, const tcp::endpoint) {
                if (ec) {
                    println("connected {}", ec.message());
                    return;
                }

                println("connected");
                recv_data();
            });
        }

        ~client() {
            _io.stop();
            if (_io_thred.joinable()) {
                _io_thred.join();
            }
        }

        void recv_data() {
            _socket.async_read_some(
            asio::buffer(data_exchange.rx_buffer),
            [this] (error_code ec, size_t len) {
                if (ec) {
                    println("recv error {}", ec.message());
                    return;
                }

                if (len != 0) {
                    std::string msg(reinterpret_cast<const char*>(data_exchange.rx_buffer.data()), len);
                    if (csv::inst()->is_csv_line(msg)) {
                        std::string csv = csv::inst()->strip_csv_prefix(msg);

                        csv::inst()->lock();
                        csv::inst()->push(csv);
                        csv::inst()->unlock();
                    }
                }

                recv_data();
            });
        }
        
    private:
        asio::io_context _io;
        std::thread _io_thred;
        tcp::socket _socket;
        tcp::resolver _resolver;

        struct {
            vector<uint8_t> rx_buffer;
            vector<uint8_t> tx_buffer;

        } data_exchange;
    };

}

#endif