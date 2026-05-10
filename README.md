# ESP32 WiFi Raw TCP Server (Port 2323)

## Objective
To implement a TCP server on ESP32 that connects to a WiFi network and communicates with a client using raw TCP instead of telnet, ensuring binary-safe data transmission.

## Background
In earlier implementations, port 23 (telnet) is commonly used for communication. However, telnet interprets certain byte values (especially `0xFF`) as control commands rather than data.

This creates problems when dealing with real devices that transmit binary data.

To solve this, the server is moved to a custom port (`2323`) and accessed using raw TCP tools like netcat.

## Problem with Telnet
- Telnet reserves `0xFF` as a control byte (IAC – Interpret As Command)
- Binary data may be modified or lost
- Not suitable for real device communication

## Solution
- Use a custom port (`2323`)
- Use raw TCP instead of telnet
- Ensure all bytes (`0x00` to `0xFF`) are transmitted unchanged

## System Overview
ESP32 connects to WiFi and acts as a TCP server.

Laptop (nc) ⇄ ESP32 (TCP Server)

## Requirements
- ESP32 development board
- ESP-IDF (v6.x)
- WiFi network
- Serial monitor (`idf.py monitor`)

## Configuration

### WiFi Credentials
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"

### Server Configuration
#define PORT 2323
#define BUFFER_SIZE 128

## Working
- ESP32 initializes WiFi in station mode
- Connects to the configured WiFi network
- Obtains an IP address
- Starts a TCP server on port 2323
- Waits for a client connection
- Receives data using `recv()`
- Sends the same data back using `send()`

## Output
On successful execution, the serial monitor shows:

Connecting to WiFi...
CONNECTED!
IP Address: 192.168.x.x
PORT: 2323
Server listening on port 2323

## Testing

### Connect from PC
nc <ESP32_IP> 2323

### Send Text Data
hello

Output:
hello

### Send Binary Data
printf '\x80\xff\x00' | nc <ESP32_IP> 2323 | xxd

Expected Output:
80 ff 00

## Observations
- Data is transmitted without modification
- No special handling of `0xFF` is required
- Server works for both text and binary data

## Conclusion
A raw TCP server on ESP32 provides reliable and binary-safe communication. Using a custom port avoids issues caused by the telnet protocol and makes the system suitable for real-world device communication.

## Future Work
- Implement WiFi-to-Serial bridge
- Support multiple clients
- Add buffering for continuous data streams
