/**
 * Author: Brittany Hughes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getword.h"

/**
 * Get word function
 * 
 * The metacharacter patterns: 
 * >>", ">", "<", ">&", ">>&", "|", "&"
 * 
 * 
 * An array from p1.c is passed in to getword()
 * We are passing in the address of the array
 * *w ponts to each address location
 * w++ increments address location
 * 
*/
int BACK_PIPEE = 0;
int getword(char *w){
   //printf("getword() called\n");
    //printf("getword has recieved = %s\n", w);
    int ch;

    int count = 0;
    int size = -1;
    
    size_t isSymbolPopulated = 0;
    size_t isCharPopulated = 0;
    
    
    char tmp[STORAGE];

    /**
     * 
     * While we have not reached EOF we will 
     * continue to get one character at a time for processing 
    */
    while((ch = getchar()) != EOF){
       // printf("getword() while loop called\n");
       // printf("getword() while loop has recieved char= %s\n", ch);
        size++;

        if(size == STORAGE-1){// fix
            ungetc(ch, stdin);
            return count;
        }
        else
        {
            //This helps get rid of the leading whitespaces
            if ((ch == ' ') && (isSymbolPopulated == 0 && isCharPopulated == 0))
            {
                continue;
            }
            else
            {
                 
                
                if(ch == '>' || ch == '<' || ch == '&' || ch == '|' || ch == '\n' || ch == ' '  || ch == '\\')
                {
                    /**
                     * If the input did not have weird characters and the next gethar() produces one
                     * If the flag is set to 1, then we know that we have a symbol and we can unget the char
                     * and return the array before grabbing it again
                    */
                    if(isCharPopulated){
                        ungetc(ch,stdin);
                        *w = '\0';
                        return count;
                    }
                    
                    
                    isSymbolPopulated == 1;
                    //printf("WEIRD CHAR RETRIEVED: [%ch] \n", ch);
                    
                    /**
                     * The following checks for characters that do not have 
                     * A pattern sequence 
                     * < | '\n'
                     * 
                    */
                    if(ch == '<')
                    {
                    
                        *w = ch;
                        
                        count++;
                        w++;
                        
                        *w = '\0';
                        
                        return count;
                    }
                    else if(ch == '|')
                    {
                         
                        *w = ch;
                        
                        count++;
                        w++;
                        
                        *w = '\0';
                        
                        return count;
                    }
                    else if (ch == '\n')
                    {
                        *w = '\0';
                        return count;
                        
                    }

                    /**
                     * This area checks for the following patterns: > , >> , >& , >>& , &
                     * 
                    */
                    else if(ch == '>')
                    { // if the arrow is > we count and store it
                        *w = ch;
                        count++;
                        
                        ch = getchar(); // we are going to peek at the next character
                        
                        if(ch == '>')
                        {
                            w++;
			                count++;
                            *w = ch;  // if the next character is >, with the previous character it creates >>, we count and store it
                            
                            ch = getchar(); // becaues we now have the pattern >> we are going to check to see if there is a &
                            
                            if(ch == '&')
                            { 
                                w++;
                                *w = ch;// if there is an &, we now have the pattern >>& and can now return it
                                
                                count++;
                                w++;
                                
                                *w = '\0';
                                return count;
                               
                            }
                            else
                            { 
                                ungetc(ch, stdin);// all we have is the pattern >>. We wll unget char and return
                                w++;
                                *w = '\0';
                                
                                return count;
                            }
                            
                        }
                        else if(ch == '&')
                        { 
                            w++;
                            *w = ch; // if the character we peek at is & it creates the >&, we count store and return it;
                            
                            count++;
                            
                            w++;
                            *w = '\0';
                            
                            return count;                       
                            
                        }
                        else
                        { 
                            ungetc(ch, stdin);// if > doesnt have any of the patterns lised above we unget char and just return >
                            w++;
                            *w = '\0';
            
                            return count;
                        }
                        
                        
                    }
                    else if(ch == '&'){ // after checking for patterns involving > if the character passed is & we return 
                        *w = ch;
                        count++;
                        w++;
                            
                        *w = '\0';
                        return count;        
                        
                    }
                    // end of else if with the > >> >>& pattrns 
                    /**
                     * If we get a \ character we need to do various checks
                     * If the next character is a meta character we need to store the character as is without the backslash
                     * 
                     * If the char is another backslash we return the backslash
                     * 
                     * If isCharPopulated is set to 0, we know that there is a \ and it is possible that meta characters might be added to the array
                     * For example: if input is Null\&void it will produce the string "Null&void".
                    */
                    else if(ch == '\\')
                    {
                        isSymbolPopulated == 0;
                        ch = getchar();
                        

                        if(ch == '>' || ch == '<' || ch == '&' || ch == '|' || ch == ' ' || ch == '$' )
                        {
                            *w = ch;
                            count++;
                            w++;
                            
                            ch = getchar();
                            
                            if(ch == '>' || ch == '<' || ch == '&' || ch == '|' ){ // EXAMPLE: \>>&" becomes TWO words (">" and ">&")
                                if(ch == '|'){
                                    BACK_PIPEE = 1;
                                    ungetc(ch,stdin);
                                    *w = '\0';
                                    return -5;
                                }
                                ungetc(ch,stdin);
                                *w = '\0';
                                return count;
                            }
                            else if(ch == '\\' ){
                                *w = ch;
                                count++;
                                w++;
                            }
                            else{
                                ungetc(ch,stdin);
                            }
                            
                        }
                        else if(ch == EOF || ch == '\n'){
                            ungetc(ch,stdin);
                            //*w = '\\';
                            //count++;
                            w++;
                        }
                        else if(ch == '\\'){
                            *w = '\\';
                            count++;
                            w++;
                        }
                         
                        else{
                            ungetc(ch,stdin);
                            
                        }
                        
                    }
                    else{
                        *w = ch;
                        count++;
                        w++;
                    }
                    
                }
                
                /**
                 * This is the area where non metacharaters that do not meet the 
                 * criteria of the above if statement are stored
                 * 
                 * NOTE: Was not able to test end
                 * 
                 * If the characters E N D . are spelled out consecutively, the program will return with -1
                 * If there is a \ present after getchar() the isCharPopulated flag is set to 0 so that the next
                 * iteration will allow it to pass through the if statement above
                 * 
                 * 
                 * 
                */
                else
                {   
                    /**
                     * This is the section that checks for end by evaluating each character individually
                     * through the use of getchar()
                     * 
                    */ 
                    if(ch == 'e' && count < 4){
                        
                        *w = ch;
                        
                        count++;
                        isCharPopulated = 1;
                        w++;
                        
                        ch = getchar();
                        
                        if(ch == 'n'){
                            
                            *w = ch;
                            count++;
                            isCharPopulated = 1;
                            w++;
                            ch = getchar();
                            
                            if(ch == 'd'){
                                *w = ch;
                                count++;
                                isCharPopulated = 1;
                                w++;
                                
                                ch = getchar();
                            
                                if(ch == '.'){
                                    *w = ch;
                                    w++;
                                    *w = '\0';
                                    
                                    return -111;
                                }
                                else{
                                    ungetc(ch, stdin);
                                  
                                    isCharPopulated = 1;
                                
                                }
                            }
                            else{
                                ungetc(ch, stdin);
                                isCharPopulated = 1;
                                
                                
                            }
                           
                        }
                        else{
                            ungetc(ch, stdin);
                            isCharPopulated = 1;
                        
                        }
                    }
                    // If end is not present we continue 
                    else{
                        *w = ch;
                        count++;
                        isCharPopulated = 1;
                        w++;
                    }
                    

                    /**
                     * This is the section that checks if we need to alter the isCharPopulated flag
                     * by checking if the next character is a \
                     * If it is, we ungetchar and change the flag to 0
                     * 
                     * If it is not we ungetchar only
                     * 
                    */
                    ch = getchar();
                    if(ch == '\\'){
                        ungetc(ch,stdin);
                        isCharPopulated = 0;
                    }
                    else{
                        ungetc(ch,stdin);
                    }
                    
                } // end of not wierd
            }
            
            
        }// end of outter else statement
             
    }//end of while
    
    /**
     * NOTE: ??? 
     * The while loop has terminated meaning we have reached EOF
     * As a result we shall send -1 to indicate the end of the file
    */

    count = -1;
    return count;
    
}

