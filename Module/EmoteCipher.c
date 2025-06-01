/**************************************************************
* File:: EmoteCipher.c
*
* Description:: Implementation of the device driver, which will
*	take in a string and translate its alphabetic letters 
*	into two character emoticons, and a message with said
*	emoticons back to alphabetical letters. It has the 
*	methods for the device driver (open, release, read,
*	write, ioctl), as well as its build and cleanup methods.
*
*	Additionally, there are methods above the device driver
*	methods that deal with the functionality of the cipher
*	aka the encoding and decoding of the driver.
*
**************************************************************/



#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>

MODULE_AUTHOR("Prince Lucky F Santos");
MODULE_DESCRIPTION("A device driver that translates alphabetical letters\n"
		   "into emoticons and vice versa");
MODULE_LICENSE("GPL");




#define EYES_LENGTH 4
#define MOUTH_LENGTH 13



#define MYMAJOR 476
#define MYMINOR 0
#define DEVICE_NAME "EmoteCipher"

#define EC_SETOFFSET 0
#define EC_SETMODE 1

#define EC_ENCODE 0

#define MAX_BUFF_SIZE 2000

/*
  Outside sources I used to create the device driver
*/
// https://tldp.org/LDP/lkmpg/2.4/html/book1.htm
// https://lwn.net/Kernel/LDD3/



/*
  ++++++++								++++++++
  ++++++++								++++++++
  ++++++++		Cipher Functionality				++++++++
  ++++++++								++++++++
  ++++++++								++++++++
*/


int findEye(char);
int findMouth(char);
int isAlpha(char);
int isAlphaLow(char);
int isAlphaUpp(char);
int makeIntoEmotes(char *,char *, int, int);
int makeIntoLetters(char *,  char *, int, int);

/*
    Contains the arrays and their sizes containing the mouth and eyes
    of the emotes to translate the alphabetic characters
*/
static const char eyes[EYES_LENGTH] = {':',';','X','B'};
static const char mouths[MOUTH_LENGTH] = {')','[','<','P','(','/','D','V',']','O',
  			                                  '|','>','3'};
static const int numEmotes = MOUTH_LENGTH * EYES_LENGTH;


/*
  ===== Helper functions for doing the encoding and decoding =====
*/

/*
  Both of these find methods will check if a character is a part
  of the mouths or eyes array; Useful for converting encoding
  message with emotes back to letters
*/
int findEye(char ch1) {
  for (int i = 0; i < EYES_LENGTH; i++) {
    if (eyes[i] == ch1) {
      return i;
    }
  }
  
  return -1;
}

int findMouth(char ch2) {
  for (int i = 0; i < MOUTH_LENGTH; i++) {
    if (mouths[i] == ch2) {
      return i;
    }
  }
  
  return -1;
}

/*
  Personal implementation of isalpha();
  Return 1 if ch is an alphabetical character regardless of case, 
  else return 0
*/
int isAlpha(char ch) {
   return  isAlphaLow(ch) || isAlphaUpp(ch) ? 1 : 0;
}

/*
  Personal implementation of islower();
  Return 1 if ch is a lowercase alphabetical character, 
  else return 0
*/
int isAlphaLow(char ch) {
   return  ('a' <= ch && ch <= 'z') ? 1 : 0;
}

/*
  Personal implementation of isupper();
  Return 1 if ch is an uppercase alphabetical character, 
  else return 0
*/
int isAlphaUpp(char ch) {
   return ('A' <= ch && ch <= 'Z') ? 1 : 0;
}



/*
  ===== Essential methods that do the encoding and decoding =====
*/

/*
  Convert the alphabetical letters in msg into emotes, and puts the
  resulting string into buffer; Returns the number of characters
  converted
*/
int makeIntoEmotes(char * msg,  char * buffer, int offset, int amount) {
  printk(KERN_INFO "| Starting makeIntoEmotes");
	int msgIdx = 0;
	int bufIdx = 0;
	
	// Convert the offset into an index to find an alphabet character
	int emoteStrt = offset % numEmotes;
  emoteStrt = emoteStrt < 0 ? (emoteStrt + numEmotes) : emoteStrt;
	
	// Used to find the emote to build in both the arrays
	int emoteIdx;
	int eyeIdx;
	int mouthIdx;
	
	for (msgIdx = 0; msgIdx < amount && bufIdx < MAX_BUFF_SIZE; msgIdx++) {
    // First, check if the char in msg now is an alphabetical letter
		if (isAlpha(msg[msgIdx])) {
		
		// Then, check if it is lowercase or uppercase to
		// determine the place of the emote
		  if (isAlphaLow(msg[msgIdx])) {
		    emoteIdx = ((msg[msgIdx] - 'a') + emoteStrt + 26) % numEmotes;
		  } else {
		    emoteIdx = ((msg[msgIdx] - 'A') + emoteStrt) % numEmotes;
		  }
      printk(KERN_INFO "|| Offset at %d = %d", msgIdx, emoteIdx);
		
		  // Then, build the emote to replace the letter ...
		  eyeIdx = emoteIdx / MOUTH_LENGTH;
		  mouthIdx = emoteIdx % MOUTH_LENGTH;
		  
		  // Put that emote in the buffer ...
		  buffer[bufIdx] = eyes[eyeIdx];
      printk(KERN_INFO "|| Eyes: %c = %c", buffer[bufIdx], eyes[eyeIdx]);
		  buffer[bufIdx + 1] = mouths[mouthIdx];
      printk(KERN_INFO "|| Mouth:%c = %c", buffer[bufIdx + 1], mouths[mouthIdx]);
		  
		  // Finally, increment the bufIdx to account for
		  // the addition of two characters
		  bufIdx += 2;
		  
		} else {
		
		  // Just put the character as is
		  buffer[bufIdx] = msg[msgIdx];

      // Go to the next char for the buffer
		  bufIdx++; 
		}
  }
	
  printk(KERN_INFO "| Ending makeIntoEmotes");
	return bufIdx;
} 



/*
  Convert the emotes inside the msg back into alphabetical letters, and
  put resulting string into buffer; Returns the number of characters
  converted
*/
int makeIntoLetters(char * msg,  char * buffer, int offset, int amount) {
  printk(KERN_INFO "| Starting makeIntoLetters");
	int msgIdx = 0;
	int bufIdx = 0;
	
	// Convert the offset into an index to find an alphabet character
	int emoteStrt = offset % numEmotes;
  emoteStrt = emoteStrt < 0 ? (emoteStrt + numEmotes) : emoteStrt;

  // For this conversion, it's easier to know about which face
  // represents the first alphabetical letter
  int firstEIdx = emoteStrt / MOUTH_LENGTH;
  int firstMIdx = emoteStrt % MOUTH_LENGTH;
  
  // Used to find the letter to build from the emote
	int eyeIdx;
	int mouthIdx;
    // Variables for smaller operations to build letter
  int tbe;            // Used for finding offset from first eyes
  int tbm;            // Used for finding offset from first mouth
  int letterOffset;   // Used to translate table offsets into letter offset


  for (bufIdx = 0; msgIdx < amount && bufIdx < MAX_BUFF_SIZE; bufIdx++) {
    // First, check if the 2 char window makes up an emote
	  eyeIdx = findEye(msg[msgIdx]);
	  mouthIdx = findMouth(msg[msgIdx + 1]);

    // Additional condition if one character is left to copy
    if (eyeIdx != -1 && mouthIdx != -1 && msgIdx < amount + 1 ) {
	    // Then, find the letter the emote is associated with
        // 1. Find the offset from first eyes
        /*
          E   F		     size - (F - E)
          0,1,2,3 | Length = 2; 4 - (2 - 0) = 2
        
          F   E		       (E - F)
          0,1,2,3 | Length = 2; 2 - 0 = 2
        */
      tbe = (firstEIdx <= eyeIdx) ? (eyeIdx - firstEIdx) :
            EYES_LENGTH - (firstEIdx - eyeIdx);
      printk(KERN_INFO "|| Relative IIdx 1: %d", tbe);
      
        // 2. Find the offset from first mouth
        /*
              E       F                    
          0,1,2,3,4,5,6,7,8,9 | Length = 6; 10 - (6 - 2) = 6

              F       E                    
          0,1,2,3,4,5,6,7,8,9 | Length = 4; 6 - 2 = 4
        */
      tbm = (firstMIdx <= mouthIdx) ? (mouthIdx - firstMIdx) :
            MOUTH_LENGTH - (firstMIdx - mouthIdx); 
      printk(KERN_INFO "|| Relative MIdx 1: %d", tbm);
      
      
      
      
        // 3. Account for first eye indexes not being 0
      if (firstEIdx > 0 && mouthIdx < firstMIdx) {
      	tbe--;
      	tbe = tbe < 0 ? EYES_LENGTH + tbe : tbe;
      }
      printk(KERN_INFO "|| Relative IIdx 2: %d", tbe);
      
      
      
      
        // 4. Convert into letter offset
      letterOffset = (tbe * MOUTH_LENGTH) + tbm;
      printk(KERN_INFO "|| Offset found: %d", letterOffset);
	          		 
	    // Set letter in buffer according to offset;
      // Higher than 25 means it is lowercase
      buffer[bufIdx] = letterOffset < 26 ? 'A' + letterOffset :
                        'a' + letterOffset - 26;
      printk(KERN_INFO "|| Char inserted into buffer from emote: %c", buffer[bufIdx]);
	          
	    // Finally, move msgIdx by 2 since two characters are converted
	    msgIdx += 2;
	          
	  } else {
	    // Just put the character as is
		  buffer[bufIdx] = msg[msgIdx];

      printk(KERN_INFO "|| Char inserted into buffer as is: %c", buffer[bufIdx]);

      // 1 character was put into buffer
	    msgIdx++;
	  }
  }
	
  printk(KERN_INFO "| Ending makeIntoLetters");
	return bufIdx;
}








/*
  ++++++++								++++++++
  ++++++++								++++++++
  ++++++++		Actual Device Driver Stuff			++++++++
  ++++++++								++++++++
  ++++++++								++++++++
*/


/*
  ===== Data structure to store information about the cipher =====
*/
struct cipherInfo {	
  // Identifier for when the module is ENCODING or DECODING 
  int mode;
  // Offset from first emote to start the alphabetic characters
  int offset;
  
  // Message to be encoded or decoded
  char * msg;
  // The length of the current message
  int msgSize;
  
  
  // Buffer to process msg into encoded/decoded form
  char * codedMsg;
  // The length of the translated message
  int codedSize;
  
} cipherInfo;

struct cdev my_cdev;  // Represents the character device driver






/*
  ===== Device driver operations (open, read, write, ioctl, and release) =====
*/




/*
  Initialize a new device file with a new data structure to store
  the offset and the mode of the cipher, as well as the buffer
  containing the user's write input
*/
static int emchp_open(struct inode * inode, struct file * fs) {

  struct cipherInfo * newCI;
  newCI = vmalloc(sizeof(struct cipherInfo));
  
  if (newCI == 0) {
    printk(KERN_ERR "Vmalloc of newCI failed. File not opened.\n");
    return -1;
  } 
  
  // Default settings are starting at offset 0 ... 
  // ... and encoding letters into emotes
  newCI->offset = 0; 
  newCI->mode = 0;
  
  // Allocate memory for the user input field;
  // + 1 to account for how makeIntoLetters checks two characters
  // in sequence
  newCI->msg = vmalloc(sizeof(char) * (MAX_BUFF_SIZE + 1));
  
  if (newCI->msg == 0) {
    printk(KERN_ERR "| Vmalloc of msg failed. File not opened.\n");
    vfree(newCI);
    return -1;
  } 
  
  // Message starts empty
  newCI->msgSize = 0;
  
  
  // Allocate memory for the translation buffer
  newCI->codedMsg = vmalloc(sizeof(char) * MAX_BUFF_SIZE);
  
  if (newCI->codedMsg == 0) {
    printk(KERN_ERR "| Vmalloc of codedMsg failed. File not opened.\n");
    vfree(newCI);
    return -1;
  }
  
  // Translation also starts empty
  newCI->codedSize = 0; 
  
  
  fs->private_data = newCI;
  
  printk(KERN_INFO "EmoteCipher Driver opened");
  return 0;
}



/*
  Free the stored data of the device driver, specifically the 
  message buffer and what is storing its data regarding the
  cipher
*/
static int emchp_close (struct inode * inode, struct file * fs) {

  struct cipherInfo * newCI;
  newCI = fs->private_data;
  
  vfree(newCI->codedMsg);
  vfree(newCI->msg);
  vfree(newCI);
  
  printk(KERN_INFO "EmoteCipher Driver closed");
  return 0;
}



/*
  Encode/Decode what is in the device driver's buffer and
  provide that as output to the user's buffer.
*/
static ssize_t emchp_read(struct file * fs, char __user * buf, 
		   size_t hsize, loff_t * off) {
  printk(KERN_INFO "Running read");

  struct cipherInfo * newCI;
  newCI = fs->private_data;
  
  // First, make sure the user isn't asking for more than
  // what the driver can offer
  int bytesToRead = hsize < MAX_BUFF_SIZE ? hsize :
                    MAX_BUFF_SIZE;
  
  // Next, make sure to only copy the translated message
  bytesToRead  = newCI->codedSize < bytesToRead ? 
                newCI->codedSize : bytesToRead;
  
  printk(KERN_INFO "| Bytes to read: %d\n", bytesToRead);
  
  // Finally, read from codedMsg and catch unread bytes
  int unread = copy_to_user(buf,newCI->codedMsg,bytesToRead);
  
  int totalRead = bytesToRead - unread;
  
  printk(KERN_INFO "Total read: %d\n", totalRead);
  
  return totalRead;
}


/*
  Write a new message to the buffer of the device driver,
  which will be translated later in read
*/
static ssize_t emchp_write(struct file * fs, const char __user * buf, 
		    size_t hsize, loff_t * off) { 
  printk(KERN_INFO "Running write");

  struct cipherInfo * newCI;
  newCI = fs->private_data;
  
  /*
    Writing original message
  */
  
  // First, make sure the user isn't going to write past
  // the driver's limists of the buffer
  int bytesToWrite = hsize < MAX_BUFF_SIZE ? hsize :
                    MAX_BUFF_SIZE;
  
  // Next, make sure to read what can be read, aka truncate
  // if needed
  bytesToWrite  = hsize < bytesToWrite ? hsize : bytesToWrite;
  
  // Finally, write to msg and catch unwritten bytes
  int unwrote = copy_from_user(newCI->msg,buf,bytesToWrite);
  printk(KERN_INFO "| Amount unwritten: %d\n", unwrote);
  
  // Now store the amount written from user input
  newCI->msgSize = bytesToWrite - unwrote;
  printk(KERN_INFO "| New size of msg: %d\n", newCI->msgSize);
  
  
  /*
    Writing the encoded/decoded version 
  */
  
  if (newCI->mode) {
    // When decoding, convert emotes into alphabet letters
    newCI->codedSize = makeIntoLetters(newCI->msg, newCI->codedMsg, 
  				      newCI->offset, newCI->msgSize);
  } else {
    // When encoding, convert alphabet letters into emotes
    newCI->codedSize = makeIntoEmotes(newCI->msg, newCI->codedMsg,
    				      newCI->offset, newCI->msgSize);
    
  }
  printk(KERN_INFO "| New size of codedMsg: %d\n\n", newCI->codedSize);
  
  // Only need what was written by user, not program
  return newCI->msgSize;
}



/*
  This ioctl method will allow the user to change how much of an offset the cipher has,
  creating a unuque manner of translation between letters and emotes, as well as the
  mode of the cipher from either encoding (letters to emotes) or decoding (vice versa)
*/
static long emchp_ioctl (struct file * fs, unsigned int command, unsigned long data) {
	printk(KERN_INFO "Running ioctl");

  struct cipherInfo * newCI;
  newCI = fs->private_data;
		 
  // Check if changing mode of encode/decode OR offset
  switch (command) {
    case EC_SETOFFSET:
      printk(KERN_INFO "Changing offset to %lu\n", data);
      newCI->offset = data;
      
      // Make sure to reset msg and codedMsg to make way
      // for new manner of output
      newCI->msgSize = 0;
      newCI->codedSize = 0;
      
      return 0;
      
    case EC_SETMODE:
      printk(KERN_INFO "Changing mode to %lu\n", data);
      newCI->mode = data;
      
      // Make sure to reset msg and codedMsg to make way
      // for new kind of input
      newCI->msgSize = 0;
      newCI->codedSize = 0;
      
      return 1;
      
    default:
      printk(KERN_ERR "Invalid value for ioctl. Mode not changed.\n");
  }
  
  
  return -1;
}



// Data structure to store the device driver operations
struct file_operations emchp_fops = {
  .open = emchp_open,
  .release = emchp_close,
  .read = emchp_read,
  .write = emchp_write,
  .unlocked_ioctl = emchp_ioctl
};






/*
  ===== Module init and cleanup =====
*/

/*
  Create a device node in /dev, return error if not made
*/
int init_module(void) {
  int result, registers;
  dev_t devno;
	
  devno = MKDEV(MYMAJOR, MYMINOR);
	
  registers = register_chrdev_region(devno,1,DEVICE_NAME);
  printk(KERN_INFO "Register chardev succeeded 1: %d\n", registers);
  cdev_init(&my_cdev,&emchp_fops);
  my_cdev.owner = THIS_MODULE;
	
  result = cdev_add(&my_cdev,devno,1);
  printk(KERN_INFO "Dev add chardev succeeded 2: %d\n", result);
  printk(KERN_INFO "Hello! Let's begin doing cool stuff! - Emote Cipher\n");
	
  if (result < 0) {
    printk(KERN_ERR "Register chdev failed: %d\n", result);
  }
	
	
  return result;
}

/*
  Unregister and remove the device from kernel
*/
void cleanup_module(void) {
  dev_t devno;
	
  devno = MKDEV(MYMAJOR, MYMINOR);
  unregister_chrdev_region(devno,1);
  cdev_del(&my_cdev);
	
  printk(KERN_INFO "Hope to see you next time! - Emote Cipher\n");
}
