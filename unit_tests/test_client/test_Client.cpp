#define UNIT_TESTING

#include <catch2/catch.hpp>
#include <fstream>
#include <string>
#include "Client.hpp"


class ClientTest : public Rat::Client {
public:
    ClientTest(const std::string& host, uint16_t port) : Rat::Client(host, port) {}
    using Rat::Client::initClientID;
    using Rat::Client::this_id_;
};

std::string createTempFile(const std::string& content) {
    char temp_path[] = "/tmp/client_id_XXXXXX";
    int fd = mkstemp(temp_path);
    REQUIRE(fd != -1);
    ssize_t written = write(fd, content.c_str(), content.size());
    REQUIRE(written == static_cast<ssize_t>(content.size()));
    close(fd);
    return temp_path;
}

TEST_CASE("Client initClientID", "[Client]") {
    ClientTest client("localhost", 8080);

    SECTION("Test valid ID file") {
        std::string temp_file = createTempFile("123456789");
        client.initClientID(temp_file);
        REQUIRE(client.this_id_ == 123456789);
        std::remove(temp_file.c_str());
    }

    SECTION("Test invalid ID file") {
        std::string temp_file = createTempFile("invalid_id");
        client.initClientID(temp_file);
        REQUIRE(client.this_id_ == static_cast<uint64_t>(-1));
        std::remove(temp_file.c_str());
    }

    SECTION("Test empty file") {
        std::string temp_file = createTempFile("");
        client.initClientID(temp_file);
        REQUIRE(client.this_id_ == static_cast<uint64_t>(-1));
        std::remove(temp_file.c_str());
    }

    SECTION("Test non-existent file") {
        client.initClientID("/tmp/non_existent_file.txt");
        REQUIRE(client.this_id_ == static_cast<uint64_t>(-1));
    }
}