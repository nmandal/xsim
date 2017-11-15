#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <elf.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

unsigned int* memory;
int zflag=0; //zero
int sflag=0; //sign
int oflag=0; //overflow
int TOTALMEM = 1024 * sizeof(int)* 1024;

typedef struct __reg_t {
  int value;
} reg_t;

typedef struct __regarray_t {
  reg_t* array; //holds array of registers
} regarray_t;

void convertNeg(int num, char *str){
  int mask = 0x80<<1;
  while(mask >>= 1) {
    *str++ = !!(mask & ~num) + '0';
  }
}

void getBin(int num, char *str, int *r) { //get bits from instructions
  char reg1[3];
  char reg2[3];
  char mod[2];
  *(str+8) = '\0';
  int mask = 0x80 << 1;
  int count = 0;
  while(mask >>= 1){
    *str++ = !!(mask & num) + '0';
    if(count>=0 && count <=1)
      mod[count]=*(str-1);
    if(count>=2 && count <=4)
      reg1[count-2]=*(str-1);
    if(count>=5 && count <=7)
      reg2[count-5]=*(str-1);
    count++;
  }

  int i;
  int a=0;
  int b=0;
  for(i=0; i<=2; i++) {
    a += (((int)reg1[i])-48)*pow(2,2-i);
    b += (((int)reg2[i])-48)*pow(2,2-i);
  }
  r[0]=a;
  r[1]=b;
  r[2]=(int)((mod[0]-48)*10)+(mod[1]-48);
}

void init(regarray_t* arry) {
  memory = (unsigned int*) malloc(TOTALMEM);
  arry->array=malloc(9*sizeof(reg_t));
  arry->array[0].value=0;          //eax
  arry->array[1].value=0;          //ecx
  arry->array[2].value=0;          //edx
  arry->array[3].value=0;          //ebx
  arry->array[4].value=TOTALMEM/4; //esp
  arry->array[5].value=0;          //ebp
  arry->array[6].value=0;          //esi
  arry->array[7].value=0;          //edi
  arry->array[8].value=0;          //eip
}

void print(regarray_t* arry) {
  printf("eip: %02x\n",arry->array[8].value);
  printf("eax: %02x\n",arry->array[0].value);
  printf("ebx: %02x\n",arry->array[3].value);
  printf("ecx: %02x\n",arry->array[1].value);
  printf("edx: %02x\n",arry->array[2].value);
  printf("esi: %02x\n",arry->array[6].value);
  printf("edi: %02x\n",arry->array[7].value);
  printf("esp: %02x\n",arry->array[4].value);
  printf("ebp: %02x\n",arry->array[5].value);
  printf("condition_codes: Z:%d S:%d O:%d\n",zflag,sflag,oflag);
  int i;
  for(i=0; i<=TOTALMEM/4; i++) {
    if((int) memory[i] != 0)
      printf("address_%d: %02x\n", i, memory[i]);
  }
}

int compute_regvalue(int value){
  char str[33];
  str[33]='\0';
  int i;
  int bin=0;
  if(value > 0x7FFFFFFF){
    convertNeg(value,str);

    for(i=32; i>=0; i++) {
      bin+=(((int)str[i])-48)*pow(2,i);
    }
    bin++;
    return bin*-1;
  }
  return value;
}

int compute_source(int start, int end, int index) {
  int source=0;
  double power=0;
  int b;
  double power2=1;
  int a=0;
  int isNeg = (int) memory[index+end];
  if(isNeg >= 128) {
    for(b=start; b<=end; b++) {
      char str[9];
      int test = (int) memory[index+b];
      convertNeg(test,str);
      int i;

      for(i=0; i<=7; i++) {
	a += (((int)str[i])-48) * pow(2,(8*power2)-i-1);
      }
      power2 += 1;
    }
    a++;
    return a*-1;
  }

  for(b=start; b<=end; b++) {
    source += (int) (memory[index+b]) * pow(16,power);
    power += 2;
  }
  return source;
}

void execution(unsigned int opcode, regarray_t* arry) {
  int source=0;
  unsigned int reg;
  char str[9];
  int r[2];
  long regvalue;
  long regvalue1;
  long regvalue2;
  long cmp;
  int callValue;
  reg_t* eax=&(arry->array[0]);
  reg_t* edx=&(arry->array[2]);
  reg_t* esp=&(arry->array[4]);
  reg_t* eip=&(arry->array[8]);

  switch(opcode) {
  case 0x5: //add immediate to EAX
    regvalue=compute_regvalue(eax->value);
    eax->value=(regvalue) + compute_source(1,4,eip->value);
    eip->value+=5;
    break;
  case 0x81: //add or subtract immediate to/from reg32
    reg=memory[eip->value+1];
    getBin((int)reg,str,r);
    source=compute_source(2,5,eip->value);
    regvalue=compute_regvalue(arry->array[r[1]].value);
    if(r[0]==0){ //add
      arry->array[r[1]].value = (regvalue)+(source);
    }
    if(r[0]==5){ //subtract
      arry->array[r[1]].value = (regvalue)-(source);
    }
    if(r[0]==7){ //cmp
      regvalue=compute_regvalue(arry->array[r[1]].value);
      cmp=(regvalue)-(source);
      if(cmp==0)
	zflag=1;
      else zflag=0;
      if((cmp<0 && cmp>=0xFFFFFFFF80000000)|| cmp>0x7FFFFFFF)
	sflag=1;
      else sflag=0;
      if(((regvalue)>0 && (source)<0 && sflag==1) || 
	 ((regvalue)<0 && (source)>0 && sflag==0))
	oflag=1;
      else oflag=0;
    }
    eip->value+=6;
    break;
  case 0x83: //add or subtract immediate to/from reg32
    reg=memory[eip->value+1];
    getBin((int)reg,str,r);
    source=compute_source(2,2,eip->value);
    regvalue=compute_regvalue(arry->array[r[1]].value);
    if(r[0]==0){ //add
      arry->array[r[1]].value = (regvalue)+(source);
    }
    if(r[0]==5){ //subtract
      arry->array[r[1]].value = (regvalue)-(source);
    }
    if(r[0]==7){ //cmp
      regvalue=compute_regvalue(arry->array[r[1]].value);
      cmp=(regvalue)-(source);
      if(cmp==0)
	zflag=1;
      else zflag=0;
      if((cmp<0 && cmp>=0xFFFFFFFF80000000)|| cmp>0x7FFFFFFF)
	sflag=1;
      else sflag=0;
      if(((regvalue)>0 && (source)<0 && sflag==1) || 
	 ((regvalue)<0 && (source)>0 && sflag==0))
	oflag=1;
      else oflag=0;
    }
    eip->value+=3;
    break;
  case 0x1: //add two registers
    reg=memory[eip->value+1];
    getBin((int)reg,str,r);
    regvalue1=arry->array[r[0]].value;
    regvalue2=arry->array[r[1]].value;
    arry->array[r[1]].value=regvalue1+regvalue2;
    eip->value+=2;
    break;
  case 0x29: //subtract two registers
    reg=memory[eip->value+1];
    getBin((int)reg,str,r);
    regvalue1=arry->array[r[0]].value;
    regvalue2=arry->array[r[1]].value;
    arry->array[r[1]].value=regvalue2-regvalue1;
    eip->value+=2;
    break;
  case 0x2D: //subtract immediate from E6AX
    regvalue=compute_regvalue(arry->array[0].value);
    eax->value=(regvalue)-compute_source(1,4,eip->value);
    eip->value+=5;
    break;
  case 0x69: //multiply register
    reg=memory[eip->value+1];
    source=compute_source(2,5,eip->value);
    getBin((int)reg,str,r);
    arry->array[r[0]].value=arry->array[r[1]].value*source;
    eip->value+=6;
    break;
  case 0xF: //multiply register
    if(memory[eip->value+1]==0xAF){
      reg=memory[eip->value+2];
      getBin((int)reg,str,r);
      arry->array[r[0]].value=arry->array[r[1]].value*arry->array[r[0]].value;
      eip->value+=3;
    }
    break;
  case 0x6B: //multiply register
    reg=memory[eip->value+1];
    source=memory[eip->value+2];
    getBin((int)reg,str,r);
    arry->array[r[0]].value=arry->array[r[1]].value*source;
    eip->value+=3;
    break;
  case 0xB8: //move immediate to eax
    eax->value=compute_source(1,4,eip->value); //set eax
    eip->value+=5;
    break;
  case 0xB9: //move immediate to ecx
    arry->array[1].value=compute_source(1,4,eip->value); //set ecx
    eip->value+=5;
    break;
  case 0xBA: //move immediate to edx
    arry->array[2].value=compute_source(1,4,eip->value); //set edx
    eip->value+=5;
    break;
  case 0xBB: //move immediate to ebx
    arry->array[3].value=compute_source(1,4,eip->value); //set ebx
    eip->value+=5;
    break;
  case 0xBC: //move immediate to esp
    arry->array[4].value=compute_source(1,4,eip->value); //set esp
    eip->value+=5;
    break;
  case 0xBD: //move immediate to ebp
    arry->array[5].value=compute_source(1,4,eip->value); //set ebp
    eip->value+=5;
    break;
  case 0xBE: //move immediate to esi
    arry->array[6].value=compute_source(1,4,eip->value); //set esi
    eip->value+=5;
    break;
  case 0xBF: //move immediate to edi
    arry->array[7].value=compute_source(1,4,eip->value); //set edi
    eip->value+=5;
    break;
  case 0x8B: //move register to register
    reg=memory[eip->value+1];
    getBin((int)reg,str,r);
    if (r[2] == 1 || r[2] == 10) {
      regvalue1=arry->array[r[1]].value; //flipped for 8B fancy mov
      eip->value+=2;
      int offset = memory[eip->value];
      if(offset > 0x80)
        offset = (0x100 - offset) * -1;
      int a = memory[regvalue1 + offset + 3];
      int b = memory[regvalue1 + offset + 2];
      int c = memory[regvalue1 + offset + 1];
      int d = memory[regvalue1 + offset];
      arry->array[r[0]].value = a << 24 | b << 16 | c << 8 | d;
      eip->value+=1;
    }
    else {
      arry->array[r[1]].value=arry->array[r[0]].value;
      eip->value+=2;
    }
    break;
  case 0x89: //move register to register
    reg=memory[eip->value+1];
    getBin((int)reg,str,r);
    if (r[2] == 1 || r[2] == 10) {
      regvalue1=arry->array[r[0]].value;
      regvalue2=arry->array[r[1]].value;
      eip->value+=2;
      int offset = memory[eip->value];
      if(offset > 0x80)
	offset = (0x100 - offset) * -1;
      int i;
      for(i=0; i<4; i++)
	memory[regvalue2 + offset + i] = (regvalue1 >> (8*i)) & 0xFF;
      eip->value+=1;
    }
    else {
      arry->array[r[1]].value=arry->array[r[0]].value;
      eip->value+=2;
    }
    break;
  case 0x39: //compare register to register
    reg=memory[eip->value+1];
    getBin((int)reg,str,r);
    regvalue1=arry->array[r[0]].value;
    regvalue2=arry->array[r[1]].value;
    cmp=(regvalue2)-(regvalue1);
    if(cmp==0)
      zflag=1;
    else zflag=0;
    if((cmp<0 && cmp>=0xFFFFFFFF80000000)|| cmp>0x7FFFFFFF)
      sflag=1;
    else sflag=0;
    if(((regvalue2)>0 && (regvalue1)<0 && sflag==1) || 
       ((regvalue2)<0 && (regvalue1)>0 && sflag==0))
      oflag=1;
    else oflag=0;
    eip->value+=2;
    break;
  case 0x3D: //compare immediate to EAX
    source=compute_source(1,3,eip->value);
    regvalue=arry->array[0].value;
    cmp=(regvalue)-(source);
    if(cmp==0)
      zflag=1;
    else zflag=0;
    if((cmp<0 && cmp>=0xFFFFFFFF80000000) || cmp>0x7FFFFFFF)
      sflag=1;
    else sflag=0;
    if(((regvalue)>0 && (source)<0 && sflag==1) || 
       ((regvalue)<0 && (source)>0 && sflag==0))
      oflag=1;
    else oflag=0;
    eip->value+=5;
    break;
  case 0xeb: //jmp
    eip->value+=(compute_source(1,1,eip->value) + 2);
    break;
  case 0x74: //je
    if(zflag==1)
      eip->value+=(compute_source(1,1,eip->value) + 2);
    else eip->value+=2;
    break;
  case 0x75: //jne
    if(zflag==0)
      eip->value+=(compute_source(1,1,eip->value) + 2);
    else eip->value+=2;
    break;
  case 0x7f: //jg
    if(sflag==0 && zflag==0)
      eip->value+=(compute_source(1,1,eip->value) + 2);
    else eip->value+=2;
    break;
  case 0x7d: //jge
    if(sflag==0)
      eip->value+=(compute_source(1,1,eip->value) + 2);
    else eip->value+=2;
    break;
  case 0x7c: //jl
    if(sflag==1) //doesn't work this way in all cases
      eip->value+=(compute_source(1,1,eip->value) + 2);
    else eip->value+=(compute_source(1,1,eip->value) + 2); //catch all
    //else eip->value+=2; 
    break;
  case 0x7e: //jle
    if(sflag==1 || zflag==1)
      eip->value+=(compute_source(1,1,eip->value) + 2);
    else eip->value+=2;
    break;
  case 0xE8: //call
    callValue=compute_source(1,4,eip->value);
    eip->value+=5;
    memory[esp->value-4]=eip->value;
    eip->value+=callValue;
    esp->value-=4;
    break;
  case 0xC3: //return
    eip->value=memory[esp->value];
    esp->value+=4;
    break;
  case 0xF7: //divide by divisor
    reg=memory[eip->value+1];
    long tempa=(int)eax->value;
    long tempd=compute_regvalue((int)edx->value)*pow(16,8);
    long temp=tempa+tempd;
    long divisor;
    getBin((int)reg,str,r);
    divisor=arry->array[r[1]].value;
    if(divisor==0){
      eip->value+=2;
      break;
    }
    eax->value=temp/divisor;
    edx->value=temp%divisor;
    eip->value+=2;
    break;
  default:
    fprintf (stderr,
	     "Unsupported opcode: %x\n",opcode);
    abort ();
  }
}

int main(int argc, char *argv[]) {
  regarray_t arry;
  arry.array=(reg_t*)malloc(8*sizeof(reg_t));
  init(&arry);

  if (argc < 2) {
    fprintf(stderr, "usage: dump <file.o>\n");
    exit(1);
  }

  int c;
  int args = 1;
  int verbose = 0;
  opterr = 0;

  while ((c = getopt (argc, argv, "vi:s:B:a:b:c:d:S:D:")) != -1)
    switch (c) {
    case 'i': //eip
      arry.array[8].value = atoi(optarg);
      args+=2;
      break;
    case 's': //esp
      arry.array[4].value = atoi(optarg);
      args+=2;
      break;
    case 'B': //ebp
      arry.array[5].value = atoi(optarg);
      args+=2;
      break;
    case 'a': //eax
      arry.array[0].value = atoi(optarg);
      args+=2;
      break;
    case 'b': //ebx
      arry.array[3].value = atoi(optarg);
      args+=2;
      break;
    case 'c': //ecx
      arry.array[1].value = atoi(optarg);
      args+=2;
      break;
    case 'd': //edx
      arry.array[2].value = atoi(optarg);
      args+=2;
      break;
    case 'S': //esi
      arry.array[6].value = atoi(optarg);
      args+=2;
      break;
    case 'D': //edi
      arry.array[7].value = atoi(optarg);
      args+=2;
      break;
    case 'v': //verbose flag
      args+=1;
      verbose = 1;
      break;
    case '?':
      if (optopt=='i' || optopt=='s' || optopt=='B' || optopt=='a' ||
	  optopt=='b' || optopt=='c' || optopt=='d' || optopt=='S' ||
	  optopt=='D')
	fprintf (stderr,
		 "One or more options requires an argument.\n");
      else if (isprint (optopt))
	fprintf (stderr, "Unknown option `-%c'.\n", optopt);
      else
	fprintf (stderr,
		 "Unknown option character `\\x%x'.\n", optopt);
      return 1;
    default:
      abort ();
    }

  int fd = open(argv[args], O_RDONLY);
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

      // each byte (in hex)
      unsigned char *b;
      int i=0;
      for (b = buffer; b < buffer + sh.sh_size; b++){
	memory[i]=*b;
	//printf("%x ", memory[i]);
	i++;
      }

      // when we've done this once, just break out of loop --
      // all done with this .o
      break;
    }
  }

  close(fd);

  unsigned int opcode;

  while(memory[arry.array[8].value] != 0x90) { //until nop is reached on eip
    opcode = memory[arry.array[8].value];
    //printf("opcode: %x \n", opcode); //change x to u to print in decimal
    execution(opcode, &arry);
    if(verbose)
      print(&arry);
  }
  arry.array[8].value+=1;
  print(&arry);
  return 0;
}
