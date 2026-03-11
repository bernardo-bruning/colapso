#include <api.h>

void main() {
    DirectoryEntry dir[16];
    get_dir(dir);

    print("--- LISTAGEM DE DISCO (C-APP) ---", 10, 20);
    
    int line = 12;
    for(int i = 0; i < 16; i++) {
        if(dir[i].active) {
            print("- ", line, 22);
            print(dir[i].name, line, 24);
            line++;
        }
    }
}
