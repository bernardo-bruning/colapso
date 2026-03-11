# Filesystem

O projeto usa um formato de disco simples e fixo.

Caracteristicas:

- imagem raw de `10 MiB`
- tabela de diretorio em `LBA 100`
- `64` entradas de `32` bytes
- leitura e escrita ATA PIO

Campos de cada entrada:

- nome ASCII com ate `15` caracteres uteis
- `start_lba`
- `sector_count`
- `is_executable`
- `active`

Convencoes:

- `is_executable = 1`: app executavel
- `is_executable = 2`: diretorio
- `is_executable = 0`: arquivo comum
