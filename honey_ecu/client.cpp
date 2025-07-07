#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread> 

std::shared_ptr<vsomeip::application> app;

void on_response(const std::shared_ptr<vsomeip::message> &response) {
    std::shared_ptr<vsomeip::payload> pl = response->get_payload();
    std::vector<vsomeip::byte_t> payload(pl->get_data(), pl->get_data() + pl->get_length());

    std::cout << "Received response: ";
    for (auto b : payload)
        std::cout << static_cast<char>(b);
    std::cout << std::endl;
}

int main() {
    app = vsomeip::runtime::get()->create_application("client");

    if (!app->init()) {
        std::cerr << " Failed to initialize client app." << std::endl;
        return 1;
    }

    app->register_message_handler(0x1234, 0x5678, 0x0421, on_response);

    app->start();

    // 연결 안정화 시간 대기
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::shared_ptr<vsomeip::message> request = vsomeip::runtime::get()->create_request();

    request->set_service(0x1234);
    request->set_instance(0x5678);
    request->set_method(0x0421);

    std::vector<vsomeip::byte_t> data = {'H', 'I'};
    std::shared_ptr<vsomeip::payload> pl = vsomeip::runtime::get()->create_payload();
    pl->set_data(data.data(), data.size());
    request->set_payload(pl);

    std::cout << " Sending request..." << std::endl;
    app->send(request);

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
