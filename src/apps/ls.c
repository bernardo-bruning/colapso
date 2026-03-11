#include <api.h>

void main() {
    DirectoryEntry dir[16];
    get_dir(dir);

    stdout_write("--- LISTAGEM DE DISCO (C-APP) ---\n");

    for(int i = 0; i < 16; i++) {
        if(dir[i].active) {
            stdout_write("- ");
            stdout_write(dir[i].name);
            stdout_write("\n");
        }
    }
}
