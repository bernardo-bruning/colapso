import struct

def create_entry(name, lba, count, is_exec):
    # Garante que o nome tenha exatamente 16 bytes terminados em zero
    name_bytes = name.encode('ascii')[:15].ljust(16, b'\x00')
    return struct.pack('16sIIII', name_bytes, lba, count, is_exec, 1)

directory_sector = bytearray(512)

# Adicionando arquivos em setores altos para segurança
entries = [
    create_entry("readme.txt", 110, 1, 0),
    create_entry("secret.txt", 111, 1, 0),
    create_entry("app", 150, 1, 1),
]

offset = 0
for entry in entries:
    directory_sector[offset:offset+32] = entry
    offset += 32

with open("build/directory.bin", "wb") as f:
    f.write(directory_sector)

with open("build/readme.bin", "wb") as f:
    f.write("Colapso OS: Disco real operando no Setor 110!".encode('ascii').ljust(512, b'\x00'))

with open("build/secret.bin", "wb") as f:
    f.write("Acesso Root: 12345".encode('ascii').ljust(512, b'\x00'))
