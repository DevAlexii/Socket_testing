import socket
import struct
import time

"""
struct ntp_packet
{
    unsigned char leap_version_mode;
    unsigned char stratum;
    unsigned char poll;
    unsigned char precision;
    unsigned int root_delay;
    unsigned int root_dispersion;
    char rfid[4];
    unsigned long long ref_timestamp;
    unsigned long long original_timestamp;
    unsigned long long receive_timestamp;
    unsigned long long transmit_timestamp;
};
"""

ntp_format = ">BBBBII4sIIIIIIII"

leap = 0
version = 4
mode = 3  # client

now = int(time.time())

ntp_packet = struct.pack(
    ntp_format,
    leap << 6 | version << 3 | mode,
    0,  # stratum
    0,  # poll
    0,  # precision
    0,  # root_delay
    0,  # root_dispersion
    b"\0\0\0\0",  # refid
    0,
    0,
    now,
    0,
    0,
    0,
    0,
    0,
)

ntp_server = ("216.239.35.12", 123)

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.sendto(ntp_packet, ntp_server)

response, sender = s.recvfrom(256)

print(sender, response, len(response))

fields = struct.unpack(ntp_format, response)

print(fields)

print(fields[0] >> 6, (fields[0] >> 3) & 0b111, fields[0] & 0b111)
print(fields[1], fields[2], fields[3])
print(fields[6])
print(fields[11], fields[12])
print(
    time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime(fields[11] - (365 * 3600 * 24 * 70)))
)