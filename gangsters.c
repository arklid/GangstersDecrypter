/*
    Copyright 2014,2015 Fredric Baeckström

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#ifdef WIN32
    #include <direct.h>
    #define PATHSLASH '\\'
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
#endif

#define VER "1.0.0"

void xtx_crypt(uint8_t *data, uint32_t datalen);
void write_output_file(char* file_output, uint32_t pos, char* buffer, uint32_t size);
void std_err(void);
uint8_t *create_dir(uint8_t *name);
void usage(char* argument);

void xtx_crypt(uint8_t *data, uint32_t datalen) {
    int v16 = datalen >> 2;
    do {
        unsigned long v17 = *(unsigned long *)data;
        data += 4;
        --v16;
        *(unsigned long *)(data - 4) = v17 ^ 0xFADEDEAF;
    } while ( v16 );
}

void write_output_file(char* file_output, uint32_t pos, char* buffer, uint32_t size) {
    FILE * pFile;
    uint8_t* tmp = create_dir((uint8_t*)file_output);

    printf("- Open output file:    %s\n", file_output);
    pFile = fopen(file_output, "wb");

    if(!pFile)
        std_err();

    if(pos)
        fseek(pFile, pos, SEEK_SET);

    fwrite(buffer, 1, size, pFile);
    fclose(pFile);
}

uint8_t *create_dir(uint8_t *name) {
    uint8_t *p,
            *l;

    for(p = name; (*p == '\\') || (*p == '/') || (*p == '.'); p++);
    name = p;

    for(;;) {
        if((p[0] && (p[1] == ':')) || ((p[0] == '.') && (p[1] == '.'))) p[0] = '_';

        l = strchr(p, '\\');
        if(!l) {
            l = strchr(p, '/');
            if(!l) break;
        }
        *l = 0;
        p = l + 1;

#ifdef WIN32
        mkdir(name);
#else
        mkdir(name, 0755);
#endif
        *l = PATHSLASH;
    }
    return(name);
}

void std_err(void) {
    perror("\nError");
    exit(1);
}

void usage(char* argument) {
    printf("Usage: %s [options] <input> <output>\n\n", argument);

    printf("Options:\n");
    printf("\t\t-d\t\t- Decrypt.\n");
    printf("\t\t-e\t\t- Encrypt.\n");
    printf("\n");
}

int main(int argc, char* argv[]) {
    uint8_t decrypt = 0;            // Decrypt flag
    uint8_t encrypt = 0;            // Encrypt flag

    uint32_t buffer_size = 0;           // Buffer size

    FILE *pFile;

    // Simple header to tell the user what this application does.
    printf("\n");
    printf("     Gangsters Organized Crime xtx decryption tool by Fredric Baeckstrom  \n");
    printf("  ======================================================= version %s ===\n\n", VER);


    // Check the number of arguments supplied, if not enough show usage to the user.
    if (argc < 4) {
        usage(argv[0]);
        return(-1);
    }


    // Parse the supplied arguments and check those against available options.
    argc -= 2;
    for(int i = 1; i < argc; i++) {
        if(((argv[i][0] != '-') && (argv[i][0] != '/')) || (strlen(argv[i]) != 2)) {
            printf("\nError: recheck your options, %s is not valid\n", argv[i]);
            exit(1);
        }

        switch(argv[i][1]) {
            case 'd':
                decrypt = 1;
                break;
            case 'e':
                encrypt = 1;
                break;

            default: {
                printf("\nError: wrong command-line argument (%s)\n\n", argv[i]);
                exit(1);
                break;
            }
        }
    }


    // Store input and output filenames.
    char* file_input = argv[argc];              // Get input filename
    char* file_output = argv[argc + 1];         // Get output filename

    printf("- Open input file:    %s\n", file_input);
    pFile = fopen(file_input, "rb");

    if(!pFile)
        std_err();

    // Retreive filesize so that we can allocate memory for our buffer.
    fseek(pFile, 0L, SEEK_END);
    buffer_size = ftell(pFile);
    fseek(pFile, 0L, SEEK_SET);

    char* buffer = (char*)malloc(buffer_size);
    memset(buffer, 0, buffer_size);     // Paranoid.

    fread(buffer, 1, buffer_size, pFile);
    fclose(pFile);

    printf("- Data size: 0x%08x  (%u)\n", buffer_size, buffer_size);

    if(decrypt)
    {
        // Decrypt the data.
        xtx_crypt((uint8_t*)buffer, buffer_size);

        // Write data back to our output file.
        write_output_file((char *)file_output, 0, (char*)buffer, buffer_size);
        printf("\n- Writing data to file, done.\n");

        // Clear memory.
        free(buffer);
    }
    else if(encrypt)
    {
        // Eecrypt the data.
        xtx_crypt((uint8_t*)buffer, buffer_size);

        // Write data back to our output file.
        write_output_file((char *)file_output, 0, (char*)buffer, buffer_size);
        printf("\n- Writing data to file, done.\n");

        // Clear memory.
        free(buffer);
    }
}
