import struct

def create_entry(name, lba, count, is_exec):
    name_field = name.encode('ascii')[:15].ljust(16, b'\x00')
    return struct.pack('16sIIII', name_field, lba, count, is_exec, 1)

directory_sector = bytearray(512)

# Organização do Disco:
# LBA 40: Bash
# LBA 60: Hello App
# LBA 100: Root Directory
entries = [
    create_entry("bash.bin", 40, 10, 1),
    create_entry("hello.bin", 60, 1, 1), 
]

offset = 0
for entry in entries:
    directory_sector[offset:offset+32] = entry
    offset += 32

with open("build/directory.bin", "wb") as f:
    f.write(directory_sector)
