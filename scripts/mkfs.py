import struct

def create_entry(name, lba, count, is_exec):
    name_field = name.encode('ascii')[:15].ljust(16, b'\x00')
    return struct.pack('16sIIII', name_field, lba, count, is_exec, 1)

directory_sector = bytearray(2048)

# Bash no Setor 200
entries = [
    create_entry("bash.bin", 200, 10, 1),
    create_entry("bin", 0, 0, 2),
    create_entry("bin/ls", 60, 2, 1),
    create_entry("bin/cat", 80, 2, 1),
    create_entry("bin/hello", 70, 2, 1),
    create_entry("bin/help", 110, 2, 1),
    create_entry("bin/echo", 120, 2, 1),
    create_entry("bin/stat", 130, 4, 1),
    create_entry("bin/hexdump", 140, 2, 1),
    create_entry("bin/uname", 150, 2, 1),
    create_entry("bin/pwd", 160, 2, 1),
    create_entry("bin/which", 170, 2, 1),
    create_entry("bin/wc", 180, 2, 1),
    create_entry("bin/grep", 190, 4, 1),
    create_entry("bin/head", 210, 2, 1),
    create_entry("bin/tail", 220, 2, 1),
    create_entry("bin/more", 230, 2, 1),
    create_entry("bin/sleep", 240, 2, 1),
    create_entry("bin/dmesg", 250, 4, 1),
    create_entry("bin/meminfo", 260, 2, 1),
    create_entry("bin/reboot", 270, 2, 1),
    create_entry("bin/shutdown", 280, 2, 1),
    create_entry("bin/true", 290, 2, 1),
    create_entry("bin/false", 300, 2, 1),
    create_entry("bin/cd", 310, 2, 1),
    create_entry("bin/mkdir", 320, 2, 1),
    create_entry("bin/rm", 330, 2, 1),
    create_entry("bin/cp", 340, 2, 1),
    create_entry("bin/mv", 350, 2, 1),
    create_entry("bin/touch", 360, 2, 1),
    create_entry("bin/write", 370, 2, 1),
    create_entry("bin/date", 380, 2, 1),
    create_entry("bin/env", 390, 2, 1),
    create_entry("bin/edit", 400, 2, 1),
    create_entry("README.txt", 90, 1, 0),
]

offset = 0
for entry in entries:
    directory_sector[offset:offset+32] = entry
    offset += 32

with open("build/directory.bin", "wb") as f:
    f.write(directory_sector)
