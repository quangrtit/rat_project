#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include "NetworkManager.hpp"

using namespace Rat;

TEST_CASE("NetworkManager send and receive", "[NetworkManager]") {
    NetworkManager nm;
    auto& io_context = nm.get_io_context();
    auto socket = std::make_shared<NetworkManager::TcpSocket>(io_context);

    SECTION("Send valid command") {
        Command command(CommandType::LIST_FILES, "test_payload");
        nm.send(socket, command, [](const NetworkManager::ErrorCode& ec) {
            REQUIRE(!ec); // Không có lỗi
        });
    }

    SECTION("Receive valid response") {
        Command command(CommandType::LIST_FILES, "test_payload");
        nm.send(socket, command, [](const NetworkManager::ErrorCode& ec) {
            REQUIRE(!ec); // Không có lỗi
        });

        nm.receive(socket, [](const Response& response, const NetworkManager::ErrorCode& ec) {
            REQUIRE(!ec); // Không có lỗi
            REQUIRE(response.success == true); // Giả định rằng lệnh thành công
        });
    }

    SECTION("Send command with oversized payload") {
        std::string oversized_payload(Rat::MAX_BUFFER_SIZE + 1, 'a'); // Tạo payload quá lớn

        // Kiểm tra rằng việc tạo lệnh với payload quá lớn ném ra ngoại lệ
        REQUIRE_THROWS_AS(Command(CommandType::LIST_FILES, oversized_payload), std::runtime_error);
    }

    SECTION("Receive response with invalid payload size") {
        // Giả lập một phản hồi không hợp lệ
        nm.receive(socket, [](const Response& response, const NetworkManager::ErrorCode& ec) {
            REQUIRE(ec); // Kiểm tra có lỗi
            REQUIRE(response.success == false); // Phản hồi không thành công
        });
    }

    // SECTION("Dequeue command from empty queue") {
    //     Command command;
    //     bool result = nm.dequeue_command(command);
    //     REQUIRE(result == false); // Không có lệnh nào trong hàng đợi
    // }

    // SECTION("Enqueue and dequeue command") {
    //     Command command(CommandType::LIST_FILES, "test_payload");
    //     nm.enqueue_command(command);

    //     Command dequeued_command;
    //     bool result = nm.dequeue_command(dequeued_command);
    //     REQUIRE(result == true); // Đã có lệnh trong hàng đợi
    //     REQUIRE(dequeued_command.type == command.type);
    //     REQUIRE(dequeued_command.payload == command.payload);
    // }
}

TEST_CASE("NetworkManager error handling", "[NetworkManager]") {
    NetworkManager nm;
    auto& io_context = nm.get_io_context();
    auto socket = std::make_shared<NetworkManager::TcpSocket>(io_context);

    SECTION("Receive response with invalid command type") {
        // Giả lập một phản hồi không hợp lệ với loại lệnh không hợp lệ
        nm.receive(socket, [](const Response& response, const NetworkManager::ErrorCode& ec) {
            REQUIRE(ec); // Kiểm tra có lỗi
            REQUIRE(response.success == false); // Phản hồi không thành công
        });
    }
}
