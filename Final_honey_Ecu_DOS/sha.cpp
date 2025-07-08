#include <vsomeip/vsomeip.hpp>
#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <openssl/sha.h>

std::shared_ptr<vsomeip::application> app;

std::vector<vsomeip::byte_t> read_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::binary);
    return std::vector<vsomeip::byte_t>((std::istreambuf_iterator<char>(file)), {});
}

std::string calculate_sha256(const std::vector<vsomeip::byte_t> &data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(data.data(), data.size(), hash);
    char output[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf(output + i * 2, "%02x", hash[i]);
    output[64] = 0;
    return std::string(output);
}

std::string read_signature(const std::string &filename) {
    std::ifstream file(filename);
    std::string sig;
    std::getline(file, sig);
    return sig;
}

void send_to_honeypot(const std::vector<vsomeip::byte_t> &data) {
    std::shared_ptr<vsomeip::message> request = vsomeip::runtime::get()->create_request();
    request->set_service(0x1234);
    request->set_instance(0x5678);
    request->set_method(0x0421);

    auto pl = vsomeip::runtime::get()->create_payload();
    pl->set_data(data);
    request->set_payload(pl);

    std::cout << "[!] 공격 탐지됨! 허니팟으로 전송 중..." << std::endl;
    app->send(request);
}

int main() {
    app = vsomeip::runtime::get()->create_application("client");

    if (!app->init()) {
        std::cerr << " Failed to initialize client app." << std::endl;
        return 1;
    }

    app->start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 1. firmware 및 서명 로드
    auto firmware = read_file("firmware.bin");
    auto expected_hash = read_signature("firmware.sig");
    auto actual_hash = calculate_sha256(firmware);

    std::cout << "[*] SHA256 검증 중...\n";
    std::cout << " - Expected: " << expected_hash << "\n";
    std::cout << " - Actual  : " << actual_hash << "\n";

    if (expected_hash != actual_hash) {
        std::cout << "[!] SHA256 불일치 → 위조 감지됨.\n";
        send_to_honeypot(firmware);
    } else {
        std::cout << "[+] 서명 일치 정상 펌웨어로 판단됨.\n";
    }

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}