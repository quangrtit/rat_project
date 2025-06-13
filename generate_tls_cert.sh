#!/bin/bash

set -e

# === Logging Function ===
log() {
    echo "[$(date +'%Y-%m-%d %H:%M:%S')] $1"
}

# === Help Option ===
if [[ "$1" == "-h" || "$1" == "--help" ]]; then
    echo "Usage: $0 [IP] [CN]"
    echo "  IP: IP address for the server certificate (default: 127.0.0.1)"
    echo "  CN: Common Name for the server certificate (default: localhost)"
    exit 0
fi

# === Configuration ===
IP=${1:-"127.0.0.1"}
CN_NAME=${2:-"localhost"}
DAYS_VALID=365
BASE_DIR=${BASE_DIR:-"/home/quang/rat_project"}
CERTS_DIR="$BASE_DIR/certs"
SERVER_CERT_DIR="$BASE_DIR/server/certs"
SERVER_BUILD_DIR="$BASE_DIR/server/build"
CLIENT_CERT_DIR="$BASE_DIR/client/certs"
CLIENT_BUILD_DIR="$BASE_DIR/client/build"

# === Output Files ===
CA_KEY="$CERTS_DIR/ca.key"
CA_CERT="$CERTS_DIR/ca.crt"
SERVER_KEY="$CERTS_DIR/server.key"
SERVER_CSR="$CERTS_DIR/server.csr"
SERVER_CERT="$CERTS_DIR/server.crt"
DH_PARAM="$CERTS_DIR/dh2048.pem"
SAN_FILE="$CERTS_DIR/san.cnf"

# === Input Validation ===
if [[ ! $IP =~ ^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$ ]]; then
    log "❌ Error: Invalid IP address format"
    exit 1
fi

log "🔐 Generating certificates with IP: $IP"

# === Step 0: Clear and recreate certs folder ===
log "🧹 Clearing existing certificates in $CERTS_DIR..."
rm -rf "$CERTS_DIR" || log "⚠️ Warning: Failed to remove $CERTS_DIR"
mkdir -p "$CERTS_DIR"

# === Step 1: Generate CA ===
log "🔧 Generating Certificate Authority (CA)..."
openssl genrsa -out "$CA_KEY" 4096
openssl req -x509 -new -nodes -key "$CA_KEY" -sha256 -days $DAYS_VALID \
    -out "$CA_CERT" -subj "/CN=MyCA"
chmod 600 "$CA_KEY"

# === Step 2: Create SAN config file ===
log "📄 Writing SAN config to $SAN_FILE..."
ALT_NAMES="DNS:localhost"
if [[ "$IP" != "127.0.0.1" ]]; then
    ALT_NAMES="$ALT_NAMES,IP:127.0.0.1,IP:$IP"
else
    ALT_NAMES="$ALT_NAMES,IP:127.0.0.1"
fi
cat > "$SAN_FILE" <<EOF
[req]
distinguished_name = req_distinguished_name
req_extensions = v3_req
[req_distinguished_name]
[v3_req]
subjectAltName = $ALT_NAMES
EOF

# === Step 3: Generate server key ===
log "🔑 Generating server key..."
openssl genrsa -out "$SERVER_KEY" 4096
chmod 600 "$SERVER_KEY"

# === Step 4: Generate server CSR ===
log "📝 Creating server CSR..."
openssl req -new -key "$SERVER_KEY" -out "$SERVER_CSR" -subj "/CN=$CN_NAME" -config "$SAN_FILE"

# === Step 5: Sign server certificate with CA ===
log "✍️ Signing server certificate..."
openssl x509 -req -in "$SERVER_CSR" -CA "$CA_CERT" -CAkey "$CA_KEY" -CAcreateserial \
    -out "$SERVER_CERT" -days $DAYS_VALID -sha256 -extensions v3_req -extfile "$SAN_FILE"

# === Step 6: Generate DH Params ===
log "🛡️ Generating DH parameters..."
openssl dhparam -out "$DH_PARAM" 2048

# === Step 7: Cleanup temporary files ===
log "🧹 Cleaning up temporary files..."
rm -f "$SERVER_CSR" "$SAN_FILE" "$CERTS_DIR/ca.srl"

# === Step 8: Distribute certificates ===
log "🚛 Distributing to server folders..."
rm -f $SERVER_CERT_DIR/*.{crt,key,pem} || log "⚠️ Warning: Failed to remove old server certificates"
rm -f $SERVER_BUILD_DIR/*.{crt,key,pem} || log "⚠️ Warning: Failed to remove old server build certificates"
mkdir -p "$SERVER_CERT_DIR" "$SERVER_BUILD_DIR"
cp "$CA_CERT" "$SERVER_CERT" "$SERVER_KEY" "$DH_PARAM" "$SERVER_CERT_DIR/"
cp "$CA_CERT" "$SERVER_CERT" "$SERVER_KEY" "$DH_PARAM" "$SERVER_BUILD_DIR/"

log "🚛 Distributing to client folders..."
rm -f $CLIENT_CERT_DIR/*.{crt,key,pem} || log "⚠️ Warning: Failed to remove old client certificates"
rm -f $CLIENT_BUILD_DIR/*.{crt,key,pem} || log "⚠️ Warning: Failed to remove old client build certificates"
mkdir -p "$CLIENT_CERT_DIR" "$CLIENT_BUILD_DIR"
cp "$CA_CERT" "$CLIENT_CERT_DIR/"
cp "$CA_CERT" "$CLIENT_BUILD_DIR/"

log "✅ Done! Certificates are ready and distributed."