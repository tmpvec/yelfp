#ifndef PPRINT_H
#define PPRINT_H

//  ELF Header
void print64Ehdr(Elf64_Ehdr* ehdr);
void print32Ehdr(Elf32_Ehdr* ehdr); 

// Program Header
void print64Phdr(Elf64_Phdr* phdr);
void print32Phdr(Elf32_Phdr* phdr);

// TODO: add Section Header Support

#endif // !PPRINT_H