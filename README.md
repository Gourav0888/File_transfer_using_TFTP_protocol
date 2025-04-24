# 📁 Remote File Transfer using TFTP Protocol (UDP)

This project implements a lightweight **Remote File Transfer** system based on the **TFTP (Trivial File Transfer Protocol)** using **UDP sockets** in C. It supports basic file transfer operations between a **client** and a **server**, simulating how embedded systems handle minimalistic file transfers.

---

## 🧠 Protocol Overview

- Based on **TFTP** – a simple file transfer protocol.
- Uses **UDP** for fast, connectionless communication.
- Follows a **stop-and-wait** approach with acknowledgments.

---

## ⚙️ Client Operations

1. **Connect** – Establish connection with server using IP and port.
2. **Put** – Upload a file to the server.
3. **Get** – Download a file from the server.
4. **Mode** – Choose one of three available transfer modes.

---

## 🔄 Supported Transfer Modes

1. **Normal Mode** – Transfers 512 bytes data per packet.
2. **Octet Mode** – Raw byte-by-byte transfer.
3. **NetASCII Mode** – Converts `\n` to `\r\n` for cross-platform compatibility.

---

## ✅ Acknowledgment & ❌ Error Handling

- After **each data packet**, the receiver sends an **ACK** back to confirm successful reception.
- If any error or packet loss is detected:
  - An **Error Packet** is sent back to the sender.
  - Sender **retransmits the last packet** to ensure data integrity.

---

## 🛠 Tech Stack

- **Language**: C  
- **Protocols**: UDP, TFTP  
- **Concepts**:
  - Socket Programming
  - File I/O
  - Packet-based communication
  - Error recovery and retransmission

---

## 🚀 Compile & Run
1. For client compilation --> `gcc tftp_client.h tftp_client.c tftp.c tftp.h -o client`.
2. For server compilation --> `gcc tftp_server.c tftp.c tftp.h -o server`.
3. Then provide --> `./client` for client and `./server` for server.


# Run Server
./server <port>

# Run Client
./client <server_ip> <port>
