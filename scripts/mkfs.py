import struct

def create_entry(name, lba, count, is_exec):
    name_field = name.encode('ascii')[:15].ljust(16, b'\x00')
    return struct.pack('16sIIII', name_field, lba, count, is_exec, 1)

directory_sector = bytearray(512)

# Bash no Setor 200
entries = [
    create_entry("bash.bin", 200, 10, 1),
    create_entry("bin/ls", 60, 2, 1),
    create_entry("bin/cat", 80, 2, 1),
    create_entry("bin/hello", 70, 2, 1),
    create_entry("README.txt", 90, 1, 0),
]

offset = 0
for entry in entries:
    directory_sector[offset:offset+32] = entry
    offset += 32

with open("build/directory.bin", "wb") as f:
    f.write(directory_sector)
