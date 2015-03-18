/**
 * Verify checksum of HSP EXE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

uint8_t file_readbyte(FILE * fp)
{
    return fgetc(fp);
}

uint16_t file_readshort(FILE * fp)
{
    uint8_t b1 = file_readbyte(fp);
    uint8_t b2 = file_readbyte(fp);
    return b1 | (b2 << 8);
}

uint32_t file_readword(FILE * fp)
{
    uint16_t s1 = file_readshort(fp);
    uint16_t s2 = file_readshort(fp);
    return s1 | (s2 << 16);
}

void file_writebyte(FILE * fp, uint8_t value)
{
    fputc(value, fp);
}

void file_writeshort(FILE * fp, uint16_t value)
{
    file_writebyte(fp, value & 0xff);
    file_writebyte(fp, (value >> 8) & 0xff);
}

void file_writeword(FILE * fp, uint32_t value)
{
    file_writeshort(fp, value & 0xffff);
    file_writeshort(fp, (value >> 16) & 0xffff);
}

bool is_portable_executable(const char * filename)
{
    // get file size
    struct stat fileinf;
    if (stat(filename, &fileinf) != 0) {
        return false;
    }
    size_t filesize = fileinf.st_size;

    // check exe size (until PE header offset)
    if (filesize < 0x40) {
        return false;
    }

    // open the exeutable file
    FILE * fp = fopen(filename, "rb");
    if (fp == NULL) {
        return false;
    }

    // check MS-DOS executable signature
    if (file_readshort(fp) != 0x5a4d) { // "MZ"
        fclose(fp);
        return false;
    }

    // get PE (Portable Executable) header offset
    fseek(fp, 0x3c, SEEK_SET);
    uint32_t offset_pe_header = file_readword(fp);
    if (offset_pe_header + 4 > filesize) {
        fclose(fp);
        return false;
    }

    // check PE header signature
    fseek(fp, offset_pe_header, SEEK_SET);
    if (file_readword(fp) != 0x4550) { // "PE\0\0"
        fclose(fp);
        return false;
    }

    return true;
}

off_t file_search_bin(FILE * fp, const char * pattern, size_t pattern_size)
{
    if (pattern_size == 0) {
        return -1;
    }

    while (feof(fp) == 0) {
        int c = fgetc(fp);
        if (c != EOF && c == pattern[0]) {
            size_t i;
            for (i = 1; i < pattern_size; i++) {
                c = fgetc(fp);
                if (c != EOF && c != pattern[i]) {
                    break;
                }
            }

            if (i == pattern_size) {
                fseek(fp, -pattern_size, SEEK_CUR);
                return (off_t) ftell(fp);
            }
            else {
                fseek(fp, i - 1, SEEK_CUR);
            }
        }
    }

    return -1;
}

off_t search_dpmx(FILE * fp, size_t filesize)
{
    rewind(fp);

    off_t offset_to_dpm;
    while ((offset_to_dpm = file_search_bin(fp, "DPMX", 4)) != -1) {
        // additional check for false-positive case (the string "DPMX" also appears in the runtime part)
        uint32_t the_word;

        // offset to data section
        the_word = file_readword(fp);
        if (feof(fp) != 0 || the_word < 0x30) {
            fseek(fp, -4 + 1, SEEK_CUR);
            continue;
        }

        // number of files
        the_word = file_readword(fp);
        if (feof(fp) != 0 || the_word == 0 || the_word >= 0x1000000) {
            fseek(fp, -8 + 1, SEEK_CUR);
            continue;
        }

        return offset_to_dpm;
    }

    return -1;
}

off_t search_hsphed(FILE * fp, size_t filesize, uint32_t offset_to_dpm)
{
    char pattern[32];
    sprintf(pattern, "%d", offset_to_dpm - 0x10000);
    size_t pattern_size = strlen(pattern);

    rewind(fp);
    return file_search_bin(fp, pattern, pattern_size);
}

void printUsage (const char * cmd)
{
    printf("Usage: %s (options) filename.exe\n", cmd);
    printf("\n");
    printf("Options\n");
    printf("-------\n");
    printf("\n");
    printf("- `-f`: Fix broken checksum if possible\n");
    printf("\n");
}

int main(int argc, char * argv[])
{
    bool verbose = false;
    bool fix_checksum = false;

    int argi = 1;
    while (argi < argc && argv[argi][0] == '-') {
        if (strcmp(argv[argi], "-f") == 0) {
            fix_checksum = true;
        }
        else if (strcmp(argv[argi], "-v") == 0 || strcmp(argv[argi], "--verbose") == 0) {
            verbose = true;
        }
        else if (strcmp(argv[argi], "--help") == 0) {
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
        else {
            printf("Error: Unknown option \"%s\"\n", argv[argi]);
        }
        argi++;
    }

    int argnum = argc - argi;
    if (argnum == 0) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argnum > 1) {
        printf("Error: Too many arguments\n");
        return EXIT_FAILURE;
    }

    const char * filename = argv[argi];

    // check PE (Portable Executable) header
    if (!is_portable_executable(filename)) {
        printf("Error: Not a Portable Executable image \"%s\"\n", filename);
        return EXIT_FAILURE;
    }

    // get file size
    struct stat fileinf;
    if (stat(filename, &fileinf) != 0) {
        printf("Error: Unable to open \"%s\"\n", filename);
        return EXIT_FAILURE;
    }
    size_t filesize = fileinf.st_size;

    // open the exeutable file
    FILE * fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error: Unable to open \"%s\"\n", filename);
        return EXIT_FAILURE;
    }

    // HSP EXE has a DPM archive, usually at the end of executable.
    // The offset to DPM archive is hardcoded to the EXE, usually in ".data" section.
    // The offset is coded as an ASCII decimal string. (For example, "176128")
    // It is a relative offset from the EXE offset 0x10000.
    // The region which holds the offset string is sometimes called HSPHED.
    // The HSPHED contains the checksum and DPM encryption keys. So we need to find it first.

    // search for DPM archive
    off_t offset_to_dpm = search_dpmx(fp, filesize);
    if (offset_to_dpm == -1) {
        printf("Error: Unable to find DPM archive in \"%s\"\n", filename);
        fclose(fp);
        return EXIT_FAILURE;
    }
    if (verbose) {
        printf("DPM archive offset: 0x%08X\n", offset_to_dpm);
    }

    // search for HSPHED
    off_t offset_to_hsphed = search_hsphed(fp, filesize, offset_to_dpm);
    if (offset_to_hsphed == -1) {
        printf("Error: Unable to find HSPHED in \"%s\" (maybe compressed?)\n", filename);
        fclose(fp);
        return EXIT_FAILURE;
    }
    if (verbose) {
        printf("HSPHED offset: 0x%08X\n", offset_to_hsphed);
    }

    // read expected checksum and key for checksum
    fseek(fp, offset_to_hsphed + 0x14, SEEK_SET);
    uint16_t checksum_in_header = file_readshort(fp);
    fseek(fp, offset_to_hsphed + 0x17, SEEK_SET);
    uint32_t crypt_key = file_readword(fp);

    // calculate the checksum from the head of DPM to EOF (see dpmread.cpp)
    uint16_t checksum = 0;
    uint32_t checksum_data_size = 0;
    fseek(fp, offset_to_dpm, SEEK_SET);
    while (feof(fp) == 0) {
        int c = fgetc(fp);
        if (c != EOF) {
            checksum += c;
            checksum_data_size++;
        }
    }
    checksum += (((crypt_key >> 24) & 0xff) / 7) * checksum_data_size;

    fclose(fp);

    if (checksum_in_header == checksum) {
        if (verbose) {
            printf("Checksum: 0x%04X\n", checksum);
        }
        else {
            printf("%04x\n", checksum);
        }
    }
    else {
        printf("Header Checksum: 0x%04X\n", checksum_in_header);
        printf("Actual Checksum: 0x%04X\n", checksum);

        if (fix_checksum) {
            // open the exeutable file
            FILE * fp = fopen(filename, "r+b");
            if (fp == NULL) {
                printf("Error: Unable to open \"%s\"\n", filename);
                return EXIT_FAILURE;
            }

            if (fseek(fp, offset_to_hsphed + 0x14, SEEK_SET) != 0) {
                printf("Error: fseek failed \"%s\"\n", filename);
                fclose(fp);
                return EXIT_FAILURE;
            }

            file_writeshort(fp, checksum);
            printf("Checksum updated\n");

            fclose(fp);
        }
    }

    return EXIT_SUCCESS;
}
