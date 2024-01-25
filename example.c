#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "smaz2.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr,
        "Usage: %s 'string to test'\n", argv[0]);
        exit(1);
    }

    unsigned char buf[256];
    unsigned long olen;

    olen = smaz2_compress(buf,sizeof(buf),(unsigned char*)argv[1],strlen(argv[1]));
    printf("Compressed length (%lu): %.02f%%\n", olen, (float)olen/strlen(argv[1])*100);
    return 0;
}
