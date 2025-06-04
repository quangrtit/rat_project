#!/bin/bash

# Đường dẫn gốc
BASE_DIR="/home/quang/rat_project"

# Danh sách các file cần xoá
rm -f "$BASE_DIR/test_clients/client0/rat_client"
rm -f "$BASE_DIR/test_clients/client0/client_id.txt"
rm -f "$BASE_DIR/test_clients/client1/rat_client"
rm -f "$BASE_DIR/test_clients/client1/client_id.txt"
rm -f "$BASE_DIR/test_clients/client2/rat_client"
rm -f "$BASE_DIR/test_clients/client2/client_id.txt" 
rm -f "$BASE_DIR/test_clients/client3/rat_client"
rm -f "$BASE_DIR/test_clients/client3/client_id.txt" 
rm -f "$BASE_DIR/test_clients/client4/rat_client"
rm -f "$BASE_DIR/test_clients/client4/client_id.txt" 

# Copy client vào từng thư mục
cp "$BASE_DIR/client/build/rat_client" "$BASE_DIR/test_clients/client0/"
cp "$BASE_DIR/client/build/rat_client" "$BASE_DIR/test_clients/client1/"
cp "$BASE_DIR/client/build/rat_client" "$BASE_DIR/test_clients/client2/"
cp "$BASE_DIR/client/build/rat_client" "$BASE_DIR/test_clients/client3/"
cp "$BASE_DIR/client/build/rat_client" "$BASE_DIR/test_clients/client4/"

# # Chạy các client (ở chế độ nền)
# (cd "$BASE_DIR/test_clients/client0" && ./rat_client &)
# sleep 1
# (cd "$BASE_DIR/test_clients/client1" && ./rat_client &)
# sleep 1
# (cd "$BASE_DIR/test_clients/client2" && ./rat_client &)
# sleep 1
# (cd "$BASE_DIR/test_clients/client3" && ./rat_client &)
# sleep 1
# (cd "$BASE_DIR/test_clients/client4" && ./rat_client &)