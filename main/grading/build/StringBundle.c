//  On my honor: 
//  
//  - I have not discussed the C language code in my program with 
//    anyone other than my instructor or the teaching assistants  
//    assigned to this course. 
//  
//  - I have not used C language code obtained from another student,  
//    the Internet, or any other unauthorized source, either modified 
//    or unmodified.   
//  
//  - If any C language code or documentation used in my program  
//    was obtained from an authorized source, such as a text book or 
//    course notes, that has been clearly noted with a proper citation 
//    in the comments of my program. 
//  
//  - I have not designed this program in such a way as to defeat or 
//    interfere with the normal operation of the grading code. 
// 
//    <Ross Manfred> 
//    <Rmm26079>
#include "StringBundle.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/***  PLACE DECLRATIONS OF ANY OTHER HELPER  ***/
/***           FUNCTIONS HERE                ***/


/**  Parses *str and creates a new StringBundle object containing the
 *   separate fields of *str.
 * 
 *   Pre:     str points to a GIS record string, properly terminated
 * 
 *   Returns: a pointer to a new proper StringBundle object
 */
StringBundle* createStringBundle(const char* const str) {
	
	//Sets delimiter to look for
	const char* lim = "|";
	
	//Avoid const attachment
	char* st = (char *) malloc((strlen(str)+1) * sizeof(char));
	
	//Initialize an empty string
	char* empty = malloc(sizeof(char));
	strcpy(empty, "");
	strcpy(st, str);
	
	//Setup for internal counter of records for GIS bundle
	int count = 0;
	int leng = strlen(st); //Size of string
	for (int i = 0; i < leng; i++)
	{
		if (st[i] == *lim)
		{
			count ++;
		}
	}
	
	//Create empty string bundle
	StringBundle* s = malloc(sizeof(StringBundle));
	
	//Allocate room for tokens
	(*s).Tokens = malloc((count+1) * sizeof(char*));
	(*s).nTokens = count + 1;
	
	//Sets up internal variables
	int n = 0; //Counter to end of string
	int a = 0; //Counter to end of array
	int size = count + 1; //Size of array
	
	//Loops through string to form GIS data pieces
	while ((a < size) & (n < leng))
	{
		//temp variable to hold GIS piece to add 
		char* temp = calloc((strlen(st) + 1), sizeof(char));
		
		//Sets counter for internal string until delim
		int i = 0;
		
		//Puts values of string into temp until delim/end of string
		while (st[n + i] != *lim && (n+i) < (int)strlen(st))
		{
			temp[i] = st[n + i];
			i++;
		}
		
		//If piece is empty fill accordingly
		if (i == 0)
		{
			(*s).Tokens[a] = malloc(sizeof(char));
			strcpy((*s).Tokens[a], empty);
			n++;
		}
		
		//Fill array with data piece
		else
		{
			temp = realloc(temp, (i+1) * sizeof(char));
			temp[i] = '\0';
			(*s).Tokens[a] = malloc((i+1) * sizeof(char));
			strcpy((*s).Tokens[a], temp);
			n += (i+1);
		}
		a++; //increment array counter
		free(temp); //free temp pointer so no mem loss
	}
	
	//Checks if we still have records to fill on right end
	if (a < size)
	{
		char* tem = malloc((strlen(st) + 1) * sizeof(char));
		strcpy(tem, empty);
		tem = realloc(tem, (1 * sizeof(char)));
		(*s).Tokens[a] = malloc((1) * sizeof(char));
		strcpy((*s).Tokens[a], tem);
		free(tem);
		a++;
	}
	free(st); //frees string copy
	free(empty); //frees empty string
	return s;
}

/**  Frees all the dynamic memory content of a StringBundle object.
 *   The StringBundle object that sb points to is NOT deallocated here,
 *   because we don't know whether that object was allocated dynamically.
 * 
 *   Pre:     *sb is a proper StringBundle object
 * 
 *   Post:    all the dynamic memory involved in *sb has been freed;
 *            *sb is proper
 */
void clearStringBundle(StringBundle* sb) {
	
   /***  COMPLETE THE IMPLEMENTATION OF THIS FUNCTION  ***/
   for (int i = 0; i < (int)(*sb).nTokens; i++)
   {
	   free((*sb).Tokens[i]); //frees each token
   }
   free((*sb).Tokens); //frees token array
}

/**  Prints a human-friendly display of the contents of a StringBuffer
 *   object to the supplied output stream.
 * 
 *   Pre:  *sb is a proper StringBundle object
 *         fp is open on an output file
 */
void printStringBundle(FILE* fp, const StringBundle* const sb) {
	
	//fprintf(fp, "There are %"PRIu32" tokens:\n", sb->nTokens);
	
	for (uint32_t idx = 0; idx < sb->nTokens; idx++) {
		fprintf(fp, "   %3"PRIu32":  [%s]\n", idx, sb->Tokens[idx]);
	}
}

/***  WRITE THE IMPLEMENTATIONS OF YOUR HELPER FUNCTIONS BELOW  ***/
