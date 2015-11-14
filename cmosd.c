/***
 *    CMOS Hacking Utility
 *    Oblivia Simplex
 * 
 *    Inspiried by an article by Endrazine
 *    and Darmawan Saljhun's book, BIOS DISASSEMBLY NINJITSU
 * 
 * compiling: gcc cmosd.c -O0 -o cmosd.o
 * usage: # ./cmosd 
 *
 ***/

#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>
#include <stdlib.h>
#include <string.h>

#define RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"
#define CLEARREST "\x1b[K"
#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71
#define PSWD_INDEX 0x38
#define CHKSUM_INDEX 0x2E
#define PSWD_LEN 6
#define DELAY 5000

int crack(char *cracked, char *enc_password, char *dicpath);
void read_cmos (unsigned char *buffer, int verbose, int lag);
int main(int argc, char **argv){

  unsigned char *buffer = calloc(0x60, sizeof(char));
  unsigned char *password = calloc(0x7, sizeof(char));
  int i;
  int dicloaded = 0;
  int verbose = 1;
  
  char *dicpath = malloc(0x20 * sizeof(char));
  int read_and_exit = 0;
  
  // put the getopt thing up here.

  char opt;

  int lag = DELAY;
  
  while ((opt = getopt(argc, argv, "t:rqd:")) != -1){
    switch (opt) {
    case 't':
      lag = atoi(optarg);
      break;
    case 'r':
      read_and_exit = 1;
      break;
    case 'q':
      verbose = 0;
      break;
    case 'd':
      strncpy(dicpath, optarg, 0x19);
      dicloaded = 1;
      break;
    default:
      break;
      //printf("USAGE MESSAGE goes here.\n");
    }
  }
  
  if (ioperm(CMOS_ADDR, 2, 1)){ // ask permission (set to 1) for ports 0x70, 0x71
    perror("ioperm");
    exit (1);
 }

  printf( RED"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n"COLOR_RESET
          "            -=oO( CMOS DEBA5E12 )Oo=- \n"
          "Please wait while we dump your CMOS parameters.\n"
          RED"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n"COLOR_RESET);
  
  read_cmos(buffer, verbose, lag);

  if (read_and_exit){
    goto finish;
  }
  if (dicloaded){
    goto cracker;
  }
  /* This part deals with the checksum. */
  char ans;
  printf("CMOS CHECKSUM @ 0x%2.2x: 0x%2.2x\n", CHKSUM_INDEX, *(buffer + CHKSUM_INDEX));
  printf("FLIP TO FORCE RESET (OR UNDO PRIOR FLIP) (y/N)? ");
  scanf("%c",&ans);
  if (ans == 'Y' || ans == 'y'){
    outb(CHKSUM_INDEX, CMOS_ADDR);
    usleep(lag);
    outb(~(*(buffer + CHKSUM_INDEX)), CMOS_DATA);
    usleep(lag);
    printf("*** "RED"CHECKSUM INVERTED! CMOS[0x%2.2x] = 0x%2.2x "COLOR_RESET" ***\n", CHKSUM_INDEX, inb(CMOS_DATA));
  }

 cracker:
  /* This part deals with the password. */
  memcpy(password, (buffer + PSWD_INDEX), 6);
  printf("\nENCRYPTED PASSWORD AT BYTES 0x%2.2x TO 0x%2.2x: ", PSWD_INDEX, PSWD_INDEX + PSWD_LEN);
  for (i = 0; i < 6; i++)
    printf("%2.2x ",*(password+i));
  printf("\n");

  if (!dicloaded){
    printf("To attempt to crack, enter path to dictionary file:\n>> ");
    scanf("%s", dicpath);
  }
  char cracked[16];

  if (crack(cracked, password, dicpath) != 0){
    printf("Sorry. The password has not been cracked.\n");
  } else {
    printf(RED"EUREKA:"COLOR_RESET" %s\n", cracked);
  }
  ///////////////////////////
 finish:

  /* Tidy things up. */
  if (ioperm(0x71, 2, 0)){  // we don't need perms anymore (set to 0)
    perror("ioperm");
    exit(1);
  }

  free(password);
  free(buffer);
  
  exit (0);
}

#define MAXLEN 16
int crack(char *cracked, char *password, char *dicpath){
  int counter = 0;
  int report_freq = 0x100;
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
  char attempt[MAXLEN];
  char encrypted[PSWD_LEN];
  char *npos;
  int i;
  while (fgets (attempt, MAXLEN, fd)) {
    if ((npos = strchr(attempt, '\n')) != NULL)
      *npos = '\0'; // strip off the trailing newline
    memset(encrypted, 0, PSWD_LEN);
    encrypt(encrypted, attempt);
    if (++counter % report_freq == 0){
      printf("\r[%d]  "RED, counter);
      for (i = 0; i < PSWD_LEN; i++)
        printf("%2.2x ",(unsigned char) encrypted[i]);
      printf (COLOR_RESET" [%s]"CLEARREST, attempt);
    }
    
    
    if (!strncmp(encrypted, password, MAXLEN)) {
      //printf(RED"\nEUREKA!\nCMOS PASSWORD: %s\n",attempt);
      printf("\n\n");
      strncpy(cracked, attempt, MAXLEN);
      return 0;
    }
  }
  //printf("\nSorry. After %d attempts, the password is still not cracked.\n",
  //         counter);
  printf("\n");
  return 1;
}

int encrypt(char *enc, char *unenc){

  // questions: what encryption algo is used?
  // what is the min and max length of the cmos pswd?
  /******* FAKE ********/
  long int rnd = random() & 0xFFFFFFFFFFFF;
  memcpy(enc, &rnd, PSWD_LEN);
  /****** END FAKE *****/
  
  return 0;
}


void read_cmos(unsigned char *buffer, int verbose, int lag){

  FILE *log = verbose? stdout : fopen("/dev/null","w") ;
  int i;

  fprintf(log, RED);
  fprintf(log, "\n       ");
  for (i = 0; i < 0x10; i++){
    if (i == 8)
      fprintf(log, " ");
    fprintf(log, "%2.1x ",i);
  }
  fprintf(log, "\n     +");
  for (i = 0; i < 49; i++)
    fprintf(log, "-");
  fprintf(log, "\n0x%2.2x | ",0);
  fprintf(log, COLOR_RESET);
  for (i = 0; i < 0x5C; i++){
    outb(i, CMOS_ADDR);  // pass address to CMOS address register (port 0x70)
    usleep(lag);      // give CMOS time to update data register
    *(buffer + i) = inb(CMOS_DATA); // read from CMOS data register (port 0x70)
    fprintf(log, "%2.2x ", *(buffer + i));
    if ((i+1) % 16 == 0){
      if ((i+1) == 0x40)
        fprintf(log, RED "*" COLOR_RESET);
      fprintf(log, RED "\n0x%2.2x | " COLOR_RESET ,i+1);
    } else if ((i+1) % 8 == 0)
      fprintf(log, " ");
  }
  fprintf(log, "\n\n");
}



