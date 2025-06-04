#!/bin/bash

# Đường dẫn gốc
BASE_DIR="/home/quang/rat_project"

rm -f "$BASE_DIR/test_server/rat_server"
rm -f "$BASE_DIR/test_server/list_client.txt"

# Copy server
cp "$BASE_DIR/server/build/rat_server" "$BASE_DIR/test_server/"

# Chạy server
(cd "$BASE_DIR/test_server" && ./rat_server)
