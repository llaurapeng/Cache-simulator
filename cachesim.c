
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




int log2(int n) {
   int r=0;
   while (n>>=1) {
       r++;
   }
  
   return r;
  
}



int main (int argc, char* argv []) {
   FILE* file;
   char  operation[1000];
   unsigned int address;
   int byte;
   unsigned long long data;
   file = fopen (argv[1], "r");


   int size= atoi (argv[2]) << 10;
   int blockSize = atoi (argv[4]);
   int ways = atoi (argv[3]);
   int setSize = size/blockSize/ ways;


   int blockOffsetSize = log2 (blockSize);
   int indexSize = log2(setSize);
   int tagSize = 24 - blockOffsetSize - indexSize;
  
   //cache setup --> columns: 2* number of ways
   //start with tag, data, block, least, valid bit used num so that's why you multiply by 5
  
   int cache [setSize][5*ways]; //setSize not 15
   int cacheWayLen = 5*ways;
   //printf ("ways %d", ways);
  
   //sets the entire cache to 0
   for (int i = 0; i<setSize; i++) { //change back to setSize
           for (int j = 0; j < cacheWayLen;j++) {
               cache[i][j]=0;
           }
       }



    int memoryLen= 1<<24;  

    int *memory = (int *)calloc(memoryLen, sizeof(int *));

    for (int i = 0; i < memoryLen; ++i) {
        memory[i] = 0;
        
    }

   //########################################################################
  int leastflag = ways;

  int loop = 0;
  int val = 0;
  int dataUpdate  = 0;
  unsigned int x = 0;

  int recentHigh[setSize];


  while (fscanf (file,"%s", operation)==1) {
      
       if (feof(file)) {
           break;
       }


       if (strcmp (operation, "store") ==0) {
           fscanf (file, "%x %d %llx", &address, &size, &data);
           

           //printf ("%s %x %d %x\n", operation, address, size, data);


       } else {
           fscanf (file, "%x %d", &address, &size);
          // printf ("%s %x %d\n", operation, address, size);
       }




       int blockOffset = address & ((1 << blockOffsetSize) - 1);
       int index = (address >> blockOffsetSize) & ((1 << indexSize) - 1);
       int tag = address >> (blockOffsetSize+ indexSize);
      
       //printy

       // printf ("block: %d\n Index: %d\nTag: %d\n\n", blockOffset, index, tag);


       int yes = 0;



/*      STORE HIT
    -check if in cache
    -change the data --> go into block
    -least bit, and the data are the only things changed
    -tag, index stay the same
    -don't really care about storing the block offset

*/

dataUpdate = 0;

  if (strcmp(operation, "store")==0) {
           printf ("%s ", "store");
           yes = 1;
           //loop through cache
           for (int i = 0; i < cacheWayLen; i+=5) {
               if (cache [index][i] == tag && cache[index][i+4]!=0) { // if the current tag equals a tag of something in the set, can't equal 0 because that means it has not been written into
                   yes = -1; //set to -1 to show that went thorugh a store hit
                   printf ("0x%x ", address);
                   printf ("%s\n", "hit");
                    
                    //high numbers mean more recent so that's why adding by 1
                    cache[index][i+3] = recentHigh[index]+1;
                   recentHigh[index] = recentHigh[index] +1;
                    val = size -1;
                    for (int i = 0; i < size; i++) {
                        x = (data >> (8*val)) & 0xff;
                        memory[address+i] = x;
                        val--;
                     }  //for end

                     i = cacheWayLen+1;

               } //if end

           } //for loop end


    } //store hit end


    //__________STORE MISS______________________________________________________________________
    /* -2 conditions: found the tag and index in memory and didn't find the and index in memory
    -found in memory then update memory and update the data

    */

   dataUpdate = 0;

    if (yes == 1) {
        printf ("0x%x ", address);
        printf ("%s\n", "miss");
        //store into MEMORY
        //printf ("data: %x\n",data);
        val = size-1;
        //char mystring [100];
        //printf ("swidata: %x",switchedData);
       //snprintf(mystring, sizeof(mystring), "%u", data);
       
       /*char str [20];
        snprintf(str, sizeof(str), "%x", data);
         printf("Using snprintf: %s\n", str);
         */

      // printf ("%d",size);
        for (int i = 0; i < size; i++) {
            x = (data >> (8* (size -1 -i))) & 0xff; //abcdefgh
            //unsigned int result = strtoul(byte, NULL, 10);
            //printf ("place: %d  val: %02x\n",address+i,x);
            memory[address+i] = x;

  
            val--;

        }

    } //yes = 1 end

                  
       //LOAD HIT CASE
        if (strcmp(operation, "load")==0) {
  
           //check the cache
           printf ("%s ", "load");
           printf ("0x%x ", address);
           yes = 10;


           for (int i = 0; i <cacheWayLen; i+=5) {
                //printf ("\ntag: %d curr tag: %d\n",cache[index][i], tag);
               //current tag equals cache and i+4 = the valid and invalid bits
               if (cache [index][i] == tag && cache[index][i+4]!=0) {
                  //printf ("tag: %d curr tag: %d\n ",cache[index][i], tag);
                   yes = 1;
                   printf ("%s ", "hit");
                   //printf ("through");

                   //update the least flag
                   cache[index][i+3] = recentHigh[index]+1;
                   recentHigh[index] = recentHigh[index] +1;
                   cache[index][i+4] = 1;
                   //printf ("data: %x\n", data);

                    val = size - 1;
                    for (int i = 0; i < size; i++) {
                        //x = (data >> (8*val)) & 0xff; //abcdefgh
                        //memory[address+i] = x;

                        printf ("%02x", memory[address+i]);
                        val--;

                    } //end of for
                    printf ("\n");

                    i = cacheWayLen+1;


                } //end of if found the right tag and index in the cache


            } // end of loop


    } //load hit end


    //LOAD MISS CASE
    /*
    -update the cache
    -load the value and print it out

    */
       if (yes == 10) {//miss load not found in cache so go to memory
           printf ("%s ", "miss");
          // printf ("here"); //delete
          val = size-1;
        //printf ("swidata: %x",switchedData);
            for (int i = 0; i < size; i++) {
                //printf ("place: %d  val: %d\n", address+i, memory[address+i]);

                printf ("%02x", memory[address+i]);
                val--;
            }

            printf ("\n");

            yes =2; 
        } //  miss load end


        //___________________UPDATE CACHE FOR LOAD MISS______________________________
        if (yes == 2) {
           //update the cache when you have a load miss
           //printf ("Update the cache\n");
           //printf ("%x",address);
           //printf ("%x",data);
           int leastVal = -10000000; //variable to find the least used column so the highest number
           int col = 0;
           int yes = 0;
           int col2 =0;
           int highNum = 167772166;
           int temp = 0;
           int stop = 0;


           for (int i = 0; i < cacheWayLen; i+=5) {

               if (loop == 0) { //
                   recentHigh[index] = 1;
               } else if (loop !=0 && cache[index][i+3] > leastVal) { //trys to find the high num
                   leastVal = cache[index][i+3];
                   col2 = i; //where the highest recent is
                   recentHigh[index] = cache[index][i+3]; //value of the highest recent
               }

               if (cache[index][i+3]  < highNum) { //used to find the spot of the lowest
                   col = i;
                   highNum = cache[index][i+3];
               }
        
               //printf ("\ntag: %d curr tag: %d on: %d" ,cache[index][i], tag,cache[index][i+4]);

               if ( cache[index][i+4]==0 && stop == 0) { // find a slot that is empty and fill the value
                    //need to save i 
                    temp = i;
                    yes =5;
                    stop =1;
                   //printf ("%x",data);


               }

           } // loop through cache ened


           if (yes == 5) { //put values in empty slot
            cache [index][temp] = tag;
            cache [index] [temp+3] = recentHigh[index]+1; //make it the most recent
            cache [index] [temp+4] =1;

          
           }

           //go here if didn't find a open slot (KICK OUT!!!!!!)


           if (yes == 0) { //put values in the kickout slot
             // printf ("Kickout Tag: %d\n", cache[index][col]); //delete
               cache[index][col] = tag;
               cache[index][col+1] = data;
               cache[index][col+3] = recentHigh[index] + 1;
               cache[index][col+4]= 1;
              // printf ("least %x\n",cache[index][col+3]);


           }
           recentHigh [index] = recentHigh[index] + 1;

       } // if yes == 2 end

       //printf ("%d\n",recentHigh);

       /*for (int i = 0; i < 5; i++) {
           for (int j = 0; j < 5; j++) {
               printf ("%x ",memory[i][j]);
           }


           printf ("\n");
       }
       */

      loop =1; //NEED THIS
        /*
       for (int i = 0; i <= 10; i++) {
           //printf ("hell");
           for (int j = 0; j < cacheWayLen; j++) {
               printf ("%x ",cache[i][j]);
           }


           printf ("\n");
          
       }
       
      
    */

   //printf ("_________\n");
    


  } //big while end

  return 0;

}// main end

