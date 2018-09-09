#include <windows.h>

#include "makedir.h"

void makeDir(const char *path){
    CreateDirectory(path, NULL);
}
