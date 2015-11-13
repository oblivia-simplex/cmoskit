/***
 *    CMOS Dumper
 *    Endrazine
 *
 * compiling: gcc cmosd.c -o cmosd.o
 * usage: # cmosd > cmod.dump
 *
 ***/

#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <stdlib.h>
#include <string.h>

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71
#define PSWD_INDEX 0x38
#define CHKSUM_INDEX 0x2E
#define PSWD_LEN 6


int crack(char *cracked, char *enc_password, char *dicpath);

int main(int argc, char **argv){

  unsigned char *buffer = calloc(0x60, sizeof(char));
  unsigned char *password = calloc(0x7, sizeof(char));
  int i;
  int dicloaded = 0;
  char *dicpath = malloc(0x20 * sizeof(char));
  
  // put the getopt thing up here.
  

  
  
  if (ioperm(CMOS_ADDR, 2, 1)){ // ask permission (set to 1) for ports 0x70, 0x71
    perror("ioperm");
    exit (1);
 }
/*
  if (0 != iopl(3)){ // try to obtain highest IO priv level
    fprintf(stderr, "FATAL ERROR: UNABLE TO OBTAIN HIGHEST IOPL\n");
    exit(EXIT_FAILURE);
  }
  
*/
  printf( "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n"
          "DUMPING BIOS PARAMETERS FROM CMOS\n"
          "This will take a moment...\n"
          "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

  printf("\n       ");
  for (i = 0; i < 0x10; i++){
    if (i == 8)
      printf(" ");
    printf("%2.1x ",i);
  }
  printf("\n     +");
  for (i = 0; i < 49; i++)
    printf("-");
  printf("\n0x%2.2x | ",0);
  for (i = 0; i < 0x5C; i++){
    outb(i, CMOS_ADDR);  // pass address to CMOS address register (port 0x70)
    usleep(100000);      // give CMOS time to update data register
    *(buffer + i) = inb(CMOS_DATA); // read from CMOS data register (port 0x70)
    printf("%2.2x ", *(buffer + i));
    if ((i+1) % 16 == 0){
      if ((i+1) == 0x40)
        printf("*");
      printf("\n0x%2.2x | ",i+1);
    } else if ((i+1) % 8 == 0)
      printf(" ");
  }
  puts("");

  memcpy(password, (buffer + PSWD_INDEX), 6);

  char ans;
  printf("CMOS CHECKSUM @ 0x%2.2x: 0x%2.2x\n", CHKSUM_INDEX, *(buffer + CHKSUM_INDEX));
  printf("RESET (y/N)? ");
  scanf("%c",&ans);
  if (ans == 'Y' || ans == 'y'){
    outb(CHKSUM_INDEX, CMOS_ADDR);
    usleep(100000);
    outb(~(*(buffer + CHKSUM_INDEX)), CMOS_DATA);
    usleep(100000);
    printf("*** CHECKSUM INVERTED! CMOS[0x%2.2x] = 0x%2.2x ***\n", CHKSUM_INDEX, inb(CMOS_DATA));
  }
  
      
  printf("\nENCRYPTED PASSWORD @ 0x%2.2x-0x%2.2x: ", PSWD_INDEX, PSWD_INDEX + PSWD_LEN);
  for (i = 0; i < 6; i++)
    printf("%2.2x ",*(password+i));
  printf("\n");
  
  if (ioperm(0x71, 2, 0)){  // we don't need perms anymore (set to 0)
    perror("ioperm");
    exit(1);
  }

  if (!dicloaded){
    printf("To attempt to crack, enter path to dictionary file:\n>> ");
    scanf("%s", dicpath);
  }
  
  char cracked[16];

  crack(cracked, password, dicpath);

  printf("CRACKED: %s\n",cracked);

  free(password);
  free(buffer);
  
  exit (0);
}

int crack(char *cracked, char *enc, char *dicpath){
  int counter = 0;
  FILE *fd;
  if ((fd = fopen(dicpath, "r")) == NULL){
    fprintf(stderr, "FAILURE TO OPEN %s\nFATAL\n",dicpath);
    exit(EXIT_FAILURE);
  }
  // basic idea: brute dictionary attack. loop through the dictionary
  // file, compare the encrypted version of the dictionary word with
  // the password. Consider: multithreading, filtering out definitely
  // wrong passwords, etc. or delegate filtering to preproc util, to
  // save MUCH time. 
  
  return 0;
}

int encrypt(char *enc, char *unenc){

  // questions: what encryption algo is used?
  // what is the min and max length of the cmos pswd?

  return 0;
}

    



