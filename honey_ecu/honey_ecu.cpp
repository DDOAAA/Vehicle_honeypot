#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>
#include <vector>

std::shared_ptr<vsomeip::application> app;
std::mutex log_mutex;

void log_request(const std::shared_ptr<vsomeip::message> &request) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ofstream log("attack_log.txt", std::ios::app);

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    log << "[!] Request received at " << std::ctime(&now);
    log << " - Client: " << request->get_client() << "\n";
    log << " - Session: " << request->get_session() << "\n";
    log << "-------------------------------\n";
}

void on_message(const std::shared_ptr<vsomeip::message> &msg) {
    std::cout << " Received message from client "
              << msg->get_client() << ", session " << msg->get_session() << std::endl;

    log_request(msg);

    auto response = vsomeip::runtime::get()->create_message(true);
    response->set_client(msg->get_client());
    response->set_session(msg->get_session());
    response->set_service(msg->get_service());
    response->set_instance(msg->get_instance());
    response->set_method(msg->get_method());

    std::shared_ptr<vsomeip::payload> payload = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> payload_data = {'O', 'K'};
    payload->set_data(payload_data.data(), payload_data.size());
    response->set_payload(payload);

    app->send(response);
}




int main() {
    app = vsomeip::runtime::get()->create_application("honey_ecu");

    if (!app->init()) {
        std::cerr << " Failed to initialize honey ECU app." << std::endl;
        return 1;
    }

    app->register_message_handler(
        0x1234,    // Service ID
        0x5678,    // Instance ID
        0x0421,    // Method ID
        on_message
    );

    app->offer_service(0x1234, 0x5678);

    std::cout << " Honey ECU started. Waiting for requests..." << std::endl;
    app->start();
}
