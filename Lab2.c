/*
 * Oscar Choy
 * ID# 53555169
 * 2/3/2017
 * Lab 2
 *
 * The program will search an array of integers to find a given integer. The array will be in a separate
 * file, to be entered in the command line when the program is started It use multiple processes to
 * search different parts of the array to find the given integer. It will recursively create two child
 * processes to search two different halves of the array. Those children will then also create two child
 * processes of their own, to search each half of the portion of the array that their parent was asked to
 * search. This splitting will continue until a child only has a single value to examine. It will then 
 * compare this element to the element it is searching for. If the integer is found, the program will 
 * then print out the array index where it was found. If it wasnt found, the program will print out a 
 * "-1" to indicate it wasn't found. This program will also use a pipe to store and transmit the values
 * of the array indexes where the integer was found by the children processes, to be read later by the
 * "master" thread.
 * 
 * This program is called like so, to search the array in a file called "arr_file.txt" for the value of 7: 
 * MySearch arr_file.txt 7
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

int parseFile(char* arrFile, int valArray[]);
int countTokens(char* arrStr);
int buildArray(char* arrStr, int valArray[]);
int mySearch(int fd[], int valArr[], int begin, int end, int find);

int main(int argc, char *argv[]) {
  int count, bytesRead, last, buf, lookFor;
  int first = 0, foundCount = 0;
  int numArray[10];
  int fd[2];
  char fileName[1000];
  pid_t pid;
  
  if (argc != 3) {
    printf("Not a valid call of MySearch. Try again.\n");
    return 0;
  } else {
    lookFor = atoi(argv[2]);
    strcpy(fileName, argv[1]);
    count = parseFile(fileName, numArray);
    if (count > 0) {
      last = count - 1;
    } else if (count == 0) {
      printf("-1\n");
      return 0;
    } else if (count == -1) {
      return -1;
    } else {
      return 0;
    }

    pipe(fd);
    mySearch(fd, numArray, first, last, lookFor);
    waitpid(-1, NULL, 0);
    close(fd[1]);
    do {
      bytesRead = read(fd[0], &buf, sizeof(buf));
      if (bytesRead > 0) {
        printf("%d ", buf);
        foundCount++;
      } 
    } while(bytesRead > 0);
    close(fd[0]);
	
    if (foundCount == 0) {
      printf("-1\n");
    } else {
      printf("\n");
    }
  }
  return 0;
}

int parseFile(char* arrFile, int valArray[]) {

  FILE *fp;
  int numToks = 0, numVals = 0;
  const char s[] = "\n";
  char line1[1000];
  char line2[1000];
  
  fp = fopen(arrFile, "r");
  if (fp == NULL) {
    perror("Error opening file");
    return -1;
  } else {
    if (fgets(line1, 1000, fp) != NULL) {
      strcpy(line2, line1);
      numToks = countTokens(line1);
      if (numToks > 10) {
        printf("Array not valid.\n");
        numVals = -1;
      } else if (numToks == 0) {
        numVals = 0;
      } else {
        numVals = buildArray(line2, valArray);
        if (numToks != numVals){
          printf("Array not valid.\n");
          numVals = -2;
        }
      } 
    } else {
      numVals = 0;
    } 
  }
  fclose(fp);
  return numVals;
}

int countTokens(char* arrStr) {

  int tokCount = 0;
  char* token;
  const char s[] = "\n";
  
  token = strtok(arrStr, s);
  token = strtok(token, " ");
  
  while (token != NULL) {
    tokCount += 1;
    token = strtok(NULL, " ");
  }
  return tokCount;
}

int buildArray(char* arrStr, int valArray[]) {
  int idx = 0, valuesValid = 1, gVal = 0, numVals = 0, temp;
  char* token;
  char junk;
  const char s[] = "\n";
  token = strtok(arrStr, s);
  token = strtok(token, " ");
  while((valuesValid != 0) && (token != NULL)) {
    gVal = sscanf(token, "%d%c", &temp, &junk);
    token = strtok(NULL, " ");
    if (gVal == 1) {
      valArray[idx] = temp;
      idx += 1;
      numVals += 1;
    } else {
      valuesValid = 0;
      numVals = -1;
    }
  }
  return numVals;
}

int mySearch(int fd[], int valArr[], int begin, int end, int find) {
  pid_t pid, pid2;
  int buf; 
  if (begin == end) {
    if (valArr[begin] == find) {
      close(fd[0]);
      buf = begin;
      write(fd[1], &buf, sizeof(buf));
    }
    exit(0);
  } else {
    pid = fork();
    if (pid == 0) {
      mySearch(fd, valArr, begin, ((begin + end)/2), find);
      waitpid(-1, NULL, 0);
      exit(0);
    } else {
      pid2 = fork();
      if (pid2 == 0) { 
        mySearch(fd, valArr, (((begin + end)/2)+1), end, find);
        waitpid(-1, NULL, 0);
        exit(0);
      }
    }
  }
}
