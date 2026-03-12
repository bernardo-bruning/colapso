#include <api.h>

void main() {
    stdout_write("Comandos disponiveis:\n");
    stdout_write("- ls [diretorio]\n");
    stdout_write("- cat <arquivo>\n");
    stdout_write("- echo <texto>\n");
    stdout_write("- pwd\n");
    stdout_write("- cd <diretorio>\n");
    stdout_write("- stat <arquivo>\n");
    stdout_write("- wc <arquivo>\n");
    stdout_write("- grep <texto> <arquivo>\n");
    stdout_write("- head <arquivo>\n");
    stdout_write("- tail <arquivo>\n");
    stdout_write("- more <arquivo>\n");
    stdout_write("- hexdump <arquivo>\n");
    stdout_write("- uname\n");
    stdout_write("- dmesg\n");
    stdout_write("- meminfo\n");
    stdout_write("- touch <arquivo>\n");
    stdout_write("- write <arquivo> <conteudo>\n");
    stdout_write("- mkdir <diretorio>\n");
    stdout_write("- edit <arquivo>\n");
    stdout_write("  ed: a i p n d w q\n");
    stdout_write("- clear\n");
}
