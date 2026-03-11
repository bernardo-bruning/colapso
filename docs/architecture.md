# Arquitetura

O Colapso OS e um sistema minimalista para x86 de 32 bits.

Camadas principais:

- `boot`: carrega o kernel, ativa A20, configura a GDT e entra em protected mode.
- `kernel`: inicializa interrupcoes, teclado, syscall e acesso basico ao disco.
- `bash`: shell simples que le comandos, resolve caminhos e carrega apps.
- `apps`: binarios pequenos carregados para `0x40000`.

Fluxo de boot:

1. O BIOS entrega o controle ao setor de boot.
2. O bootloader carrega o kernel da imagem.
3. O kernel inicializa a infraestrutura minima.
4. O shell e carregado e assume a interacao com o usuario.
