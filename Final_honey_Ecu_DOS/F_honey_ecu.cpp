#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <cstring> // for memcpy

std::shared_ptr<vsomeip::application> app;
std::mutex log_mutex;

const vsomeip::service_t service_id = 0x1234;
const vsomeip::instance_t instance_id = 0x5678;
const vsomeip::method_t method_id = 0x0421;

#pragma pack(push, 1)
struct custom_payload_t {
    uint32_t a;
    float b[2];
    uint32_t d;
    float e[2];
    uint8_t f;
};
#pragma pack(pop)

void log_payload(const custom_payload_t& data, vsomeip::client_t client_id, vsomeip::session_t session_id) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::ofstream log("F_attack_log.txt", std::ios::app);

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    log << "[!] Request received at " << std::ctime(&now);
    log << " - Client: " << client_id << "\n";
    log << " - Session: " << session_id << "\n";
    log << " - Payload: a=" << data.a
        << ", b[0]=" << data.b[0]
        << ", b[1]=" << data.b[1]
        << ", d=" << data.d
        << ", e[0]=" << data.e[0]
        << ", e[1]=" << data.e[1]
        << ", f=" << static_cast<int>(data.f) << "\n";
    log << "-------------------------------\n";
}

void on_message(const std::shared_ptr<vsomeip::message> &msg) {
    std::cout << "[honey_ecu] Message received from client "
              << msg->get_client() << ", session " << msg->get_session() << std::endl;

    // Payload 처리
    auto pl = msg->get_payload();
    if (pl && pl->get_length() == sizeof(custom_payload_t)) {
        custom_payload_t received_data;
        std::memcpy(&received_data, pl->get_data(), sizeof(received_data));
        log_payload(received_data, msg->get_client(), msg->get_session());
    } else {
        std::cout << "[honey_ecu] Invalid payload size or empty." << std::endl;
    }

    // 응답 생성
    auto response = vsomeip::runtime::get()->create_message(true);
    response->set_client(msg->get_client());
    response->set_session(msg->get_session());
    response->set_service(msg->get_service());
    response->set_instance(msg->get_instance());
    response->set_method(msg->get_method());

    auto response_payload = vsomeip::runtime::get()->create_payload();
    std::vector<vsomeip::byte_t> data = {'O', 'K'};
    response_payload->set_data(data.data(), data.size());
    response->set_payload(response_payload);

    app->send(response);
}

int main() {
    app = vsomeip::runtime::get()->create_application("honey_ecu");

    if (!app->init()) {
        std::cerr << "Failed to initialize honey_ecu app." << std::endl;
        return 1;
    }

    app->register_message_handler(service_id, instance_id, method_id, on_message);
    app->offer_service(service_id, instance_id);

    std::cout << "[honey_ecu] Started. Waiting for requests..." << std::endl;
    app->start();
}
