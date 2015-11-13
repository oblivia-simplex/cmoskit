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

int crack(char *cracked, char *enc_password, char *dicpath);

int main(int argc, char **argv){

  unsigned char *buffer = calloc(0x60, sizeof(char));
  unsigned char *password = calloc(0x7, sizeof(char));
  int i;
  int dicloaded = 0;
  char *dicpath = malloc(0x20 * sizeof(char));
  
  // put the getopt thing up here.
  

  
  if (ioperm(0x70, 2, 1)){ // ask permission (set to 1) for ports 0x70, 0x71
    perror("ioperm");
    exit (1);
  }

  printf( "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n"
          "DUMPING BIOS PARAMETERS FROM CMOS\n"
          "This will take a moment...\n"
          "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");

  printf("\n       ");
  for (i = 0; i < 0x10; i++){
    if (i == 8)
      printf(" ");
    printf("%2.2x ",i);
  }
  printf("\n     +");
  for (i = 0; i < 49; i++)
    printf("-");
  printf("\n0x%2.2x | ",0);
  for (i = 0; i < 0x5C; i++){
    outb(i, 0x70); // write to port 0x70
    usleep(100000);
    *(buffer + i) = inb(0x71);
    printf("%2.2x ", *(buffer + i));
    if ((i+1) % 16 == 0){
      if ((i+1) == 0x40)
        printf("*");
      printf("\n0x%2.2x | ",i+1);
    } else if ((i+1) % 8 == 0)
      printf(" ");
  }
  puts("");

  memcpy(password, (buffer+0x38), 6);

  printf("\nENCRYPTED PASSWORD @ 0x38-0x44: ");
  for (i = 0; i < 7; i++)
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

    



