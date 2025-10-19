#include "app.hpp"
#include <asio.hpp>
#include <print>

#include "csv.hpp"
#include "rtt_client.hpp"

int main(int argc, char**argv) {
    if (argc != 3 && argc != 1) {
        println("Error bad usage");
        println("Example usage:");
        println("\trtt-client [ip](default:127.0.0.1) [port](default:9000)");
        println("\trtt-client [ip] [port]");
        return -1;
    }

    std::string ip = "127.0.0.1";
    std::string port = "9000";

    if (argc == 3) {
        ip = argv[1];
        port = argv[2];
    }

    rtt::client client(ip, port);

    app::init();
    app::run();

    return 0;
}