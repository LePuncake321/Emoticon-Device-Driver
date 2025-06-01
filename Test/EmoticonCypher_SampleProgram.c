/**************************************************************
* File:: Santos_PrinceLucky_Hw6_main.c
*
* Description:: Test Program for the Emote Cipher Device Driver 
*
*   When this program is started, it will ask the user
*   what mode to run (Encode means to convert alphabetical
*   characters into emoticons, and decode means to translate
*   emoticons back to alphabetical characters). This is also
*   where the user can exit, where they put any input other
*   than 0 or 1.
*
*   Next, the program asks for an offset, akin to a Caesar 
*   Cypher. Note that to encode and decode a message without
*   changing it, the offset must be the same on both instances.
*   The program can also end here if the input is not an integer.
*
*   Finally, the program asks for the message to either encode
*   or decode. After this, the program keeps repeating until
*   the user inputs anything invalid on either the mode or
*   the offset.
*
**************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    int fd;
    
    fd = open("/dev/EmoteCipher", O_RDWR);
    printf("Opening EmoteCipher with %d\n", fd);
    
    if (fd < 0) {
      printf("Failed to open EmoteCipher\n");
      perror("EmoteCipher Open File ERROR\n");
      return -1;
    } else {
      printf("==== Hello there, This is the beginning of the Test Program! ====\n");
    }
    
    
    int mode = -1;
    int offset = 0;
    char input[1001];
    char output[2001];
    
    long n1, n2, n3, n4;
    long length = 0;

    while (mode != 2) {

      /*
        Before everything, Flush string output for the sake taking in new output
      */
      for (int i = 0; i < 2001; i++) {
        output[i] = '\0';
      }

      printf("==== Start of run ====\n");

      /*
        First, get all input needed
      */


      
      // Reset mode
      mode = -1;

      // Get valid mode number
      printf("++ Choose mode (0 = Encode, 1 = Decode): ");
      if (scanf("%d", &mode) != 1 || mode < 0 || mode > 1) {
      	printf("++ End of run : EXIT in mode ++\n\n");
        break;
      }
      printf("-- Mode = %d\n", mode);
      
      
      
      // Get valid offset number
      printf("++ Choose offset: ");
      if (scanf("%d", &offset) != 1) {
      	printf("++ End of run : EXIT in offset ++\n\n");
        break;
      }
      printf("-- Offset = %d\n", offset);
      
      // Get message string
// https://sekrit.de/webdocs/c/beginners-guide-away-from-scanf.html

          // Clears stdin before calling fgets
// https://www.geeksforgeeks.org/getchar-function-in-c/
      while (getchar() != '\n');

// https://www.geeksforgeeks.org/fgets-gets-c-language/
      printf("++ Write your message:\n\n");
      fgets(input,sizeof(input),stdin);

          // Make sure not to pass the nextline character
// https://www.geeksforgeeks.org/strcspn-in-c/
      input[strcspn(input, "\n")] = 0;


      /*
        Now utilize the device driver
      */

      n1 = ioctl(fd, 0, offset);
      n2 = ioctl(fd, 1, mode);
      printf("\n-- Result of ioctls: %ld, %ld\n",n1,n2);
      
      
      n3 = write(fd, input, strlen(input));
      n4 = read(fd, output, 2001);
      printf("-- Result of write and read: %ld, %ld\n\n",n3,n4);
      
      printf("-- Translated message:\n\n%s\n\n", output);
      printf("-- Translated message's length: %ld\n\n", strlen(output));  
      
      printf("++ End of run : SUCCESS ++\n\n");
    }
    
    
    
    close(fd);
    
    printf("==== Goodbye, This is the end of the Test Program! ====\n");
    return 0;
}

