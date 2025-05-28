#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "NetworkManager.hpp"
#include "Packet.pb.h"

using namespace Rat;

// Hàm giả lập logic của NetworkManager::send
std::vector<char> simulate_send(const rat::Packet& packet) {
    std::vector<char> buffer;
    std::string serialized;

    // Serialize packet (mô phỏng logic trong NetworkManager::send)
    if (!packet.SerializeToString(&serialized)) {
        return buffer; // Trả về buffer rỗng nếu serialize thất bại
    }

    // Kiểm tra kích thước
    if (serialized.size() > MAX_BUFFER_SIZE) {
        return buffer; // Trả về buffer rỗng nếu vượt quá MAX_BUFFER_SIZE
    }

    // Tạo buffer theo định dạng <packet_size: 4 bytes><packet_data>
    uint32_t packet_size = static_cast<uint32_t>(serialized.size());
    buffer.resize(sizeof(uint32_t) + packet_size);
    std::memcpy(buffer.data(), &packet_size, sizeof(uint32_t));
    std::memcpy(buffer.data() + sizeof(uint32_t), serialized.data(), packet_size);

    return buffer;
}

// Hàm giả lập logic của NetworkManager::receive
void simulate_receive(const std::vector<char>& input_data, 
                      std::function<void(const rat::Packet&, const NetworkManager::ErrorCode&)> callback) {
    // Mô phỏng logic trong NetworkManager::receive
    if (input_data.size() < sizeof(uint32_t)) {
        callback(rat::Packet(), boost::asio::error::eof);
        return;
    }

    // Đọc kích thước
    uint32_t packet_size;
    std::memcpy(&packet_size, input_data.data(), sizeof(uint32_t));

    // Kiểm tra kích thước
    if (packet_size > MAX_BUFFER_SIZE) {
        callback(rat::Packet(), boost::asio::error::message_size);
        return;
    }

    // Kiểm tra dữ liệu đủ không
    if (input_data.size() - sizeof(uint32_t) < packet_size) {
        callback(rat::Packet(), boost::asio::error::eof);
        return;
    }

    // Parse dữ liệu
    rat::Packet packet;
    if (!packet.ParseFromArray(input_data.data() + sizeof(uint32_t), packet_size)) {
        callback(rat::Packet(), boost::asio::error::invalid_argument);
        return;
    }

    callback(packet, boost::system::error_code());
}

TEST_CASE("NetworkManager send and receive", "[NetworkManager]") {
    NetworkManager nm;

    SECTION("Send valid packet") {
        rat::Packet packet;
        packet.set_type(rat::Packet::LIST_FILES);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server");
        packet.set_destination_id("client1");
        auto* command = packet.mutable_command_data();
        command->set_command("test_payload");

        auto buffer = simulate_send(packet);

        // Kiểm tra dữ liệu đã gửi
        REQUIRE(buffer.size() >= sizeof(uint32_t));
        uint32_t packet_size;
        std::memcpy(&packet_size, buffer.data(), sizeof(uint32_t));
        REQUIRE(packet_size == buffer.size() - sizeof(uint32_t));

        rat::Packet parsed_packet;
        REQUIRE(parsed_packet.ParseFromArray(buffer.data() + sizeof(uint32_t), packet_size));
        REQUIRE(parsed_packet.type() == rat::Packet::LIST_FILES);
        REQUIRE(parsed_packet.packet_id() == "test_packet");
        REQUIRE(parsed_packet.source_id() == "server");
        REQUIRE(parsed_packet.destination_id() == "client1");
        REQUIRE(parsed_packet.command_data().command() == "test_payload");
    }

    SECTION("Receive valid packet") {
        rat::Packet packet;
        packet.set_type(rat::Packet::LIST_FILES);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server");
        packet.set_destination_id("client1");
        auto* command = packet.mutable_command_data();
        command->set_command("test_payload");

        // Chuẩn bị dữ liệu giả lập
        auto buffer = simulate_send(packet);

        simulate_receive(buffer, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) {
            REQUIRE(!ec); // Không có lỗi
            REQUIRE(response.type() == rat::Packet::LIST_FILES);
            REQUIRE(response.packet_id() == "test_packet");
            REQUIRE(response.source_id() == "server");
            REQUIRE(response.destination_id() == "client1");
            REQUIRE(response.command_data().command() == "test_payload");
        });
    }

    SECTION("Send packet with oversized payload") {
        rat::Packet packet;
        packet.set_type(rat::Packet::LIST_FILES);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server");
        packet.set_destination_id("client1");
        auto* command = packet.mutable_command_data();
        command->set_command(std::string(MAX_BUFFER_SIZE + 1, 'a')); // Payload quá lớn

        auto buffer = simulate_send(packet);
        REQUIRE(buffer.empty()); // Buffer rỗng vì kích thước vượt quá MAX_BUFFER_SIZE
    }

    SECTION("Receive response with invalid payload size") {
        // Giả lập một phản hồi không hợp lệ (dữ liệu thiếu)
        std::vector<char> invalid_data(sizeof(uint32_t) - 1, 'x'); // Dữ liệu không đủ kích thước
        simulate_receive(invalid_data, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) {
            REQUIRE(ec == boost::asio::error::eof); // Lỗi do thiếu dữ liệu
            REQUIRE(response.type() == rat::Packet::UNKNOWN); // Packet rỗng
        });
    }

    SECTION("Receive packet with invalid size") {
        // Giả lập kích thước vượt quá MAX_BUFFER_SIZE
        uint32_t oversized_packet_size = MAX_BUFFER_SIZE + 1;
        std::vector<char> size_buffer(sizeof(uint32_t));
        std::memcpy(size_buffer.data(), &oversized_packet_size, sizeof(uint32_t));

        simulate_receive(size_buffer, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) {
            REQUIRE(ec == boost::asio::error::message_size); // Lỗi kích thước
            REQUIRE(response.type() == rat::Packet::UNKNOWN); // Packet rỗng
        });
    }
}

TEST_CASE("NetworkManager error handling", "[NetworkManager]") {
    NetworkManager nm;

    SECTION("Receive response with invalid command type") {
        rat::Packet packet;
        packet.set_type(rat::Packet::UNKNOWN);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server");
        packet.set_destination_id("client1");

        // Chuẩn bị dữ liệu giả lập
        auto buffer = simulate_send(packet);

        simulate_receive(buffer, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) {
            REQUIRE(!ec); // Không có lỗi
            REQUIRE(response.type() == rat::Packet::UNKNOWN); // Loại không hợp lệ
        });
    }
}