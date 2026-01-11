#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct initrd_file_header {
    char magic[8];
    uint32_t nheaders;
} __attribute__((packed));

struct initrd_file_headers {
    char name[64];
    uint32_t offset;
    uint32_t length;
} __attribute__((packed));

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <output_file> <file1> [file2] ...\n", argv[0]);
        return 1;
    }

    int nheaders = argc - 2;
    struct initrd_file_header header;
    struct initrd_file_headers *headers = (struct initrd_file_headers *)malloc(sizeof(struct initrd_file_headers) * nheaders);

    strncpy(header.magic, "INITRD", 6);
    header.magic[6] = '\0';
    header.magic[7] = '\0';
    header.nheaders = nheaders;

    printf("Creating initrd with %d files...\n", nheaders);

    unsigned int offset = sizeof(struct initrd_file_header) + sizeof(struct initrd_file_headers) * nheaders;

    for (int i = 0; i < nheaders; i++) {
        char *file_name = argv[i + 2];
        printf("Adding %s...\n", file_name);

        FILE *stream = fopen(file_name, "rb");
        if (stream == 0) {
            printf("Error: file not found: %s\n", file_name);
            return 1;
        }

        fseek(stream, 0, SEEK_END);
        long file_size = ftell(stream);
        fseek(stream, 0, SEEK_SET);

        strncpy(headers[i].name, file_name, 64);
        headers[i].offset = offset;
        headers[i].length = file_size;
        offset += file_size;
        fclose(stream);
    }

    FILE *wstream = fopen(argv[1], "wb");
    if (wstream == 0) {
        printf("Error: could not open output file: %s\n", argv[1]);
        return 1;
    }

    fwrite(&header, sizeof(header), 1, wstream);
    fwrite(headers, sizeof(struct initrd_file_headers), nheaders, wstream);

    for (int i = 0; i < nheaders; i++) {
        char *file_name = argv[i + 2];
        FILE *stream = fopen(file_name, "rb");
        fseek(stream, 0, SEEK_END);
        long file_size = ftell(stream);
        fseek(stream, 0, SEEK_SET);

        char *buf = (char *)malloc(file_size);
        fread(buf, 1, file_size, stream);
        fwrite(buf, 1, file_size, wstream);
        fclose(stream);
        free(buf);
    }

    fclose(wstream);
    free(headers);

    printf("Done.\n");

    return 0;
}
