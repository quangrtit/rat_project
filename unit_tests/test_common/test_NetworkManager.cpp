#include <catch2/catch.hpp>
#include "NetworkManager.hpp"
using namespace Rat;

std::vector<char> simulate_send(const rat::Packet& packet) 
{
    std::vector<char> buffer;
    std::string serialized;

    if (!packet.SerializeToString(&serialized)) 
    {
        return buffer; 
    }

    if (serialized.size() > MAX_BUFFER_SIZE) 
    {
        return buffer; 
    }

    uint32_t packet_size = static_cast<uint32_t>(serialized.size());
    buffer.resize(sizeof(uint32_t) + packet_size);
    std::memcpy(buffer.data(), &packet_size, sizeof(uint32_t));
    std::memcpy(buffer.data() + sizeof(uint32_t), serialized.data(), packet_size);

    return buffer;
}

void simulate_receive(const std::vector<char>& input_data, 
                      std::function<void(const rat::Packet&, const NetworkManager::ErrorCode&)> callback) 
                      {

    if (input_data.size() < sizeof(uint32_t)) 
    {
        callback(rat::Packet(), boost::asio::error::eof);
        return;
    }

    uint32_t packet_size;
    std::memcpy(&packet_size, input_data.data(), sizeof(uint32_t));

    if (packet_size > MAX_BUFFER_SIZE) 
    {
        callback(rat::Packet(), boost::asio::error::message_size);
        return;
    }

    if (input_data.size() - sizeof(uint32_t) < packet_size) 
    {
        callback(rat::Packet(), boost::asio::error::eof);
        return;
    }

    rat::Packet packet;
    if (!packet.ParseFromArray(input_data.data() + sizeof(uint32_t), packet_size)) 
    {
        callback(rat::Packet(), boost::asio::error::invalid_argument);
        return;
    }

    callback(packet, boost::system::error_code());
}

TEST_CASE("NetworkManager send and receive", "[NetworkManager]") 
{
    NetworkManager nm;

    SECTION("Send valid packet") 
    {
        rat::Packet packet;
        packet.set_type(rat::Packet::LIST_FILES_FOLDERS);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server_0");
        packet.set_destination_id("client_1");
        packet.set_encrypted(false);

        auto* chunk = packet.mutable_chunked_data();
        chunk->set_data_id("cmd_001");
        chunk->set_sequence_number(0);
        chunk->set_total_chunks(1);
        chunk->set_payload("test_payload"); 
        chunk->set_success(true);           

        auto buffer = simulate_send(packet);

        REQUIRE(buffer.size() >= sizeof(uint32_t));
        uint32_t packet_size;
        std::memcpy(&packet_size, buffer.data(), sizeof(uint32_t));
        REQUIRE(packet_size == buffer.size() - sizeof(uint32_t));

        rat::Packet parsed_packet;
        REQUIRE(parsed_packet.ParseFromArray(buffer.data() + sizeof(uint32_t), packet_size));
        REQUIRE(parsed_packet.type() == rat::Packet::LIST_FILES_FOLDERS);
        REQUIRE(parsed_packet.packet_id() == "test_packet");
        REQUIRE(parsed_packet.source_id() == "server_0");
        REQUIRE(parsed_packet.destination_id() == "client_1");

        const auto& parsed_chunk = parsed_packet.chunked_data();
        REQUIRE(parsed_chunk.data_id() == "cmd_001");
        REQUIRE(parsed_chunk.sequence_number() == 0);
        REQUIRE(parsed_chunk.total_chunks() == 1);
        REQUIRE(parsed_chunk.payload() == "test_payload");
        REQUIRE(parsed_chunk.success() == true);
    }

    SECTION("Receive valid packet") 
    {
        rat::Packet packet;
        packet.set_type(rat::Packet::LIST_FILES_FOLDERS);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server_0");
        packet.set_destination_id("client_1");
        packet.set_encrypted(false);

        auto* chunked = packet.mutable_chunked_data();
        chunked->set_data_id("data123");
        chunked->set_sequence_number(0);
        chunked->set_total_chunks(1);
        chunked->set_payload("test_payload");  
        chunked->set_success(true);
        chunked->set_error_message("");

        auto buffer = simulate_send(packet);

        simulate_receive(buffer, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) 
        {
            REQUIRE(!ec); // Không có lỗi
            REQUIRE(response.type() == rat::Packet::LIST_FILES_FOLDERS);
            REQUIRE(response.packet_id() == "test_packet");
            REQUIRE(response.source_id() == "server_0");
            REQUIRE(response.destination_id() == "client_1");

            // Kiểm tra chunked_data payload
            REQUIRE(response.chunked_data().payload() == "test_payload");
            REQUIRE(response.chunked_data().success() == true);
            REQUIRE(response.chunked_data().error_message().empty());
        });
    }

    SECTION("Send packet with oversized payload") 
    {
        rat::Packet packet;
        packet.set_type(rat::Packet::LIST_FILES_FOLDERS);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server_0");
        packet.set_destination_id("client_1");
        packet.set_encrypted(false);

        auto* chunked = packet.mutable_chunked_data();

        std::string large_payload(MAX_BUFFER_SIZE + 1, 'a');

        chunked->set_data_id("data123");
        chunked->set_sequence_number(0);
        chunked->set_total_chunks(1);
        chunked->set_payload(large_payload);
        chunked->set_success(true);
        chunked->set_error_message("");

        auto buffer = simulate_send(packet);

        REQUIRE(buffer.empty()); 
    }


    SECTION("Receive response with invalid payload size") 
    {
     
        std::vector<char> invalid_data(sizeof(uint32_t) - 1, 'x'); 
        simulate_receive(invalid_data, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) 
        {
            REQUIRE(ec == boost::asio::error::eof); 
            REQUIRE(response.type() == rat::Packet::UNKNOWN);
        });
    }

    SECTION("Receive packet with invalid size") 
    {
        uint32_t oversized_packet_size = MAX_BUFFER_SIZE + 1;
        std::vector<char> size_buffer(sizeof(uint32_t));
        std::memcpy(size_buffer.data(), &oversized_packet_size, sizeof(uint32_t));

        simulate_receive(size_buffer, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) 
        {
            REQUIRE(ec == boost::asio::error::message_size); 
            REQUIRE(response.type() == rat::Packet::UNKNOWN); 
        });
    }
}

TEST_CASE("NetworkManager error handling", "[NetworkManager]") 
{
    NetworkManager nm;

    SECTION("Receive response with invalid command type") 
    {
        rat::Packet packet;
        packet.set_type(rat::Packet::UNKNOWN);
        packet.set_packet_id("test_packet");
        packet.set_source_id("server");
        packet.set_destination_id("client1");
        packet.set_encrypted(false);

        auto buffer = simulate_send(packet);

        simulate_receive(buffer, [](const rat::Packet& response, const NetworkManager::ErrorCode& ec) 
        {
            // REQUIRE(!ec); 
            if(ec){
                // ???
            }
            REQUIRE(response.type() == rat::Packet::UNKNOWN);
        });
    }
}
