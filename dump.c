#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <elf.h>

int main(int argc, char *argv[]) {
  unsigned char *address_space;
  int bytes = (1024 * 1024);
  int register_size = 4;

  unsigned zero_flag = 0;
  unsigned sign_flag = 0;
  unsigned overflow_flag = 0;

  address_space = (char *) calloc(bytes, sizeof(char));
  assert(address_space != NULL);

  char *eax = (char *) calloc(4, sizeof(char));
  assert(eax != NULL);

  char *ebx = (char *) calloc(4, sizeof(char));
  assert(ebx != NULL);

  char *ecx = (char *) calloc(4, sizeof(char));
  assert(ecx != NULL);

  char *edx = (char *) calloc(4, sizeof(char));
  assert(edx != NULL);

  char *esi = (char *) calloc(4, sizeof(char));
  assert(esi != NULL);
  char *edi = (char *) calloc(4, sizeof(char));
  assert(edi != NULL);

  char *esp = (char *) malloc(4 * sizeof(char));
  assert(esp != NULL);

  char *ebp = (char *) calloc(4, sizeof(char));
  assert(ebp != NULL);

  char *eip = (char *) calloc(4, sizeof(char));
  assert(ebp != NULL);


  *esp = address_space[bytes - 1];

  int c;

  while ((c = getopt(argc, argv, "isBabcdSDv: ")) != -1) {
    switch(c)
    {
      case 'i':
        // initial value of instruction pointer

        break;
      case 's':
        // initial value of stack pointer

        break;

      case 'B':
        // initial value of base pointer
        break;
      case 'a':
        // initial value of eax

        break;
      case 'b':
        // initial value of ebx

        break;
      case 'c':
        // initial value of ecx

        break;
      case 'd':
        // initial value of edx

        break;
      case 'S':
        // inital value of esi

        break;
      case 'D':
        // inital value of edi

        break;
      case 'v':
      // verbose flag

        break;
      default:
        abort();
      }
    }


    if (argc != 2) {
      fprintf(stderr, "usage: dump <file.o>\n");
      exit(1);
    }

    int fd = open(argv[1], O_RDONLY);
    assert(fd >= 0);

    // read in elf header
    Elf32_Ehdr elf;
    int rc = pread(fd, &elf, sizeof(Elf32_Ehdr), 0);
    assert(rc == sizeof(elf));
    assert(elf.e_ident[0] == ELFMAG0);
    assert(elf.e_ident[1] == ELFMAG1);
    assert(elf.e_ident[2] == ELFMAG2);
    assert(elf.e_ident[3] == ELFMAG3);

    unsigned int off;
    Elf32_Shdr sh;

    int i;
    for (i = 0, off = elf.e_shoff; i < elf.e_shnum; i++, off += sizeof(sh)) {
      rc = pread(fd, &sh, sizeof(sh), off);
      assert(rc == sizeof(sh));

      // look for code (SHT_PROGBITS) that has a non-zero size: a hack but good enough for now
      if (sh.sh_type == SHT_PROGBITS && sh.sh_size > 0) {
         // now make space on heap for entire code segment
         unsigned char *buffer = malloc(sh.sh_size);
         assert(buffer != NULL);

         // read segment into buffer
         rc = pread(fd, buffer, sh.sh_size, sh.sh_offset);
         assert(rc == sh.sh_size);

         // starting address
         printf("starting address: %x\n", sh.sh_addr);

         // print out each byte (in hex)
         unsigned char *b;
         int j;
         for (b = buffer, j = 0; b < buffer + sh.sh_size; b++, j++) {
           address_space[j] = *b;
         }

         for (int k = 0; k < j; k++) {
           printf("%x ", address_space[k]);

           if (address_space[k] == 0x90) {

            printf("eip: %x\neax: %x\nebx: %x\necx: %x\nedx: %x\nesi: %x\nedi: %x\nesp: %x\nebp: %x\n", *eip, *eax, *ebx, *ecx, *edx, *esi, *edi, *esp, *ebp);
            printf("condition_codes: Z:%d S:%d O:%d\n", zero_flag, sign_flag, overflow_flag);
            //TODO print all NON-ZERO bytes in memory
            // printf("address %x\n", );
          }

        }

        // when we've done this once, just break out of loop --
        // all done with this .o
        break;
      }
    }

    close(fd);
    return 0;
}
