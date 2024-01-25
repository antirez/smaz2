#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "smaz2.h"

int main(void) {
    unsigned char in[512];
    unsigned char out[4096];
    unsigned char d[4096];
    int comprlen, decomprlen;
    int j, ranlen;
    int times = 100000;
    char *strings[] = {
        "This is a small string",
        "foobar",
        "the end",
        "not-a-g00d-Exampl333",
        "Smaz2 is a simple compression library",
        "Nothing is more difficult, and therefore more precious, than to be able to decide",
        "When words in the table are used business internet however",
        "1000!! numbers 2000?!~~ and special...characters",
        "and now a few italian sentences:",
        "Nel mezzo del cammin di nostra vita, mi ritrovai in una selva oscura",
        "Mi illumino di immenso",
        "L'autore di questa libreria vive in Sicilia",
        "You shouldn’t connect through Bluetooth, it should connect when you open the app",
        "try it against urls",
        "http://google.com",
        "http://programming.reddit.com",
        "http://github.com/antirez/smaz/tree/master",
        "/media/hdb1/music/Alben/The Bla",
        NULL
    };

    j=0;
    while(strings[j]) {
        int comprlevel;

        comprlen = smaz2_compress(out,sizeof(out),(unsigned char*)strings[j],strlen(strings[j]));
        comprlevel = 100-((100*comprlen)/strlen(strings[j]));
        decomprlen = smaz2_decompress(d,sizeof(d),out,comprlen);
        if (strlen(strings[j]) != (unsigned)decomprlen ||
            memcmp(strings[j],d,decomprlen))
        {
            printf("BUG: error compressing '%s'\n", strings[j]);
            printf("COMPRESSED TO: ");
            for (int j = 0; j < comprlen; j++)
                if (isprint(out[j]))
                    printf("%c",out[j]);
                else
                    printf("[%02x]",out[j]);
            printf("\n");
            printf("DECOMPRESSED TO: '%.*s'\n", (int)decomprlen, d);
            exit(1);
        }
        if (comprlevel < 0) {
            printf("'%s' enlarged by %d%%\n",strings[j],-comprlevel);
        } else {
            printf("'%s' compressed by %d%%\n",strings[j],comprlevel);
        }
        j++;
    }
    printf("Compressing and decompressing %d test strings...\n", times);
    while(times--) {
        char charset[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvxyz/. ";
        ranlen = random() % 512;

        for (j = 0; j < ranlen; j++) {
            if (times & 1)
                in[j] = charset[random() % (sizeof(charset)-1)];
            else
                in[j] = (char)(random() & 0xff);
        }
        comprlen = smaz2_compress(out,sizeof(out),in,ranlen);
        decomprlen = smaz2_decompress(d,sizeof(d),out,comprlen);

        if (ranlen != decomprlen || memcmp(in,d,ranlen)) {
            printf("Bug! TEST NOT PASSED\n");
            exit(1);
        }
        if (times % 10000 == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf("TEST PASSED :)\n");
    return 0;
}
