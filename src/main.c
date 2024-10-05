#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <elf.h>
#include <errno.h>
#include <sys/mman.h>

#include "pprint.h"

#define SIZE32 sizeof(Elf32_Ehdr)
#define SIZE64 sizeof(Elf64_Ehdr) 

// Throughout the program only basic error- and typechecking is provided.
// TODO: add more rigorous arch/endianness checking

int main(int argc, char** argv)
{
    // take file as input
    // if incorrect argc -> usage
    if (argc < 2)
    {
        printf("Usage: %s <filepath>\n", argv[0]);
    }
    else
    {
        printf("================\nCOOLASCIIHERE\n================\n");
    }

    // get file stats
    // check if size > x86 ELF (minimum supported)
    char *filename = argv[1];
    struct stat stbuf = {0};
    if(stat(filename, &stbuf) == 0)
    {
        printf("File size: %jd\n", stbuf.st_size);
        if ((size_t)stbuf.st_size < SIZE32)
        {
            printf("Insufficient min ELF (x86) file size. Exiting...\n");
            goto cleanup;
            return 1;
        }
    }

    // open fd for mmap
    int fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        printf("File open fail. Errno: %d.\nTo look up more about this error see errno(1)\nExiting...\n", errno);
    }
    

    // map file to memory
    void* mmFile = NULL;
    mmFile = mmap(NULL, (size_t)stbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    
    // check arch and create proper Ehdr*
    Elf64_Ehdr* elf64Ehdr = {0};
    Elf32_Ehdr* elf32Ehdr = {0};
    unsigned int archByte = ((unsigned char*)mmFile)[4]; 
    switch (archByte)
    {
    case ELFCLASSNONE:
        printf("Arch Byte Invalid. Exiting...\n");
        break;
    case ELFCLASS32:
        printf("32-bit Arch.\n");
        elf32Ehdr = (Elf32_Ehdr*)mmFile;
        break;
    case ELFCLASS64:
        printf("64-bit Arch\n");
        elf64Ehdr = (Elf64_Ehdr*)mmFile;
        break;
    default:
        printf("Arch Byte Invalid but not 0. Exiting...\n");
        break;
    }
    
    // check magic
    unsigned char magic[] = {'\x7f', '\x45', '\x4C', '\x46'};
    for (int i = 0; i < sizeof(magic)/sizeof(magic[i]); i++)
    {
        if ((elf64Ehdr ? elf64Ehdr->e_ident[i] : elf32Ehdr->e_ident[i]) != magic[i])
        {
            printf("Not an ELF file. Exiting...\n");
            goto cleanup;
            return 1;
        }
    }
    
    printf("\n================\nELF Header\n================\n");
    
    // Ehdr set using during ArchByte check
    // print elf header
    elf64Ehdr? print64Ehdr(elf64Ehdr) : print32Ehdr(elf32Ehdr);

    printf("\n================\nProgram Headers\n================\n");

    // instantiate Phdr (?)
    Elf64_Phdr* elf64Phdr = {0};
    Elf32_Phdr* elf32Phdr = {0};

    //print program headers
    for (int i = 0; i < elf64Ehdr->e_phnum; ++i) {
    elf64Phdr = (Elf64_Phdr*)((char*)mmFile + elf64Ehdr->e_phoff + (elf64Ehdr->e_phentsize * i));
    printf("PROGRAM HEADER: 0x%02x |──────────┐\n", i);
    print64Phdr(elf64Phdr);
    }

    // TODO: add Section Header Support

    cleanup:
    if (munmap(mmFile, (size_t)stbuf.st_size) == -1) {
    printf("[x] Failed to unmap header from memory\n");
    }
    
    if(close(fd) == -1)
    {
        printf("Close failed. Errno: %d.\nExiting...\n", errno);
    }

    return 0;
}



