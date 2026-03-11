import struct

def create_entry(name, lba, count, is_exec):
    # Força exatamente 16 bytes, limpando lixo
    name_bytes = name.encode('ascii')
    name_field = name_bytes[:15].ljust(16, b'\x00')
    return struct.pack('16sIIII', name_field, lba, count, is_exec, 1)

directory_sector = bytearray(512)

# Bash no Setor 40
entries = [
    create_entry("bash.bin", 40, 10, 1) 
]

offset = 0
for entry in entries:
    directory_sector[offset:offset+32] = entry
    offset += 32

with open("build/directory.bin", "wb") as f:
    f.write(directory_sector)
