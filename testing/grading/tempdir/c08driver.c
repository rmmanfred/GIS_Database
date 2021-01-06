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
/**
 * Extended use of 501 based on assumption no line 
 * is longer than 500 characters + '\0'
 **/
#include <stdio.h>
#include <stdlib.h>
#include "StringHashTable.h"
#include "StringBundle.h"
#include "FIDouble.h"
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#define PI 3.14159265358979323846 //pi
#define RADIUS 6371.0088 //radius Earth, km
//FILE *In, *Out //Make them internal
StringHashTable* featIndex; //Index table for features
FIDouble* FIndex; //Index table for FID
int Fsize; //Recorded size of FID index
int tableSZ; //Recorded size of feat index

void fcloseall();
static void dataIndexer(FILE *Data);
static void exists(char* feature, char* state, FILE *Out);
static void distance_between(int first, int second, FILE *Data, 
	FILE *Out);
static void details_of(char* feature, char* state, FILE *Out, FILE *In);
static uint32_t elfhash(const char* str);
static int compare(const void * a, const void * b);
static void commandScanner(FILE *Out, FILE *In);
static double orthodrome(double lat1, double long1, 
	double lat2, double long2);
static double toRadii(double degrees);
static FIDouble FIDlookup(int FID);
void printGIS(FILE *Out, const char* const gis, _Bool lat);
void printDir(FILE *Out, char dir);
static char* nameState(StringBundle* record);
void printLine(FILE* Out);

/**
 * Main function executes on program call, reads in inputs to find files
 *  for program
 * Pre: argc is number of arguments from command line
 * 			argv is array of strings holding arguments on command line
 * Returns: 0, if program executes correctly
 * Post: Program should create indices from provided data file and
 * 			perform search functions enumerated in script file
 * 				all posting output in output file
 **/
int main(int argc, char** argv)
{
	//Watch for correct uage of File
	//In is command file
	//Out is log file
	//Both opened and closed
	if (argc != 3)
	{
		printf("wrong number of arguments: %s\n", argv[0]);
		exit(1);
	}
	FIndex = calloc(256, sizeof(FIDouble));
	FILE *Out, *In;
	In = fopen(argv[1], "r");
	if (In == NULL)
	{
		printf("Input File Does Not Exist\n");
		fcloseall();
		exit(1);
	}
	Out = fopen(argv[2], "w");
	
	commandScanner(Out, In);
	
	StringHashTable_clear(featIndex);
	free(FIndex);
	free(featIndex);
	fclose(In);
	fclose(Out);
	fcloseall();
	return 0;
}

/**
 * Takes the data in the corresponding files and builds indices needed
 * 	for search functions
 * Pre: Out is output file, In is file containing the data to index
 * Returns: Nothing, fills universal indices with values
 **/
static void dataIndexer(FILE *Data)
{
	//malloc'd: Findex, contents of StringHashTable
	//freed: tbd in another function
	
	featIndex = StringHashTable_create(tableSZ, elfhash);
	int max = 256;
	Fsize = 0;
	uint32_t offset = 0;
	
	char line[501] = "\0";
	_Bool first = true; //skips first line
	while (fgets(line, 501, Data) != NULL)
	{
		//malloc'd: key, temp, contents of entry
		//freed: key, temp, contents of entry
		if (!first)
		{
			StringBundle* entry = createStringBundle(line);
			FIDouble* temp = malloc(sizeof(FIDouble));
			
			char* key = calloc(501, sizeof(char));
			key = strcpy(key, (*entry).Tokens[1]);
			key = strcat(key, ", ");
			key = strcat(key, (*entry).Tokens[3]);
			key = realloc(key, ((strlen(key) + 1) * sizeof(char)));
			(*temp).FID = (uint32_t)atoi((*entry).Tokens[0]);
			(*temp).offset = offset;
			StringHashTable_addEntry(featIndex, key, offset);
			
			clearStringBundle(entry);
			free(entry);
			free(key); //Not needed?
			FIndex[Fsize] = *temp;
			Fsize++;
			if (Fsize >= max)
			{
				max = max*2;
				FIndex = realloc(FIndex, max * sizeof(FIDouble));
			}
			free(temp); //////////
		}
		offset += (strlen(line));
		first = false;
	}
	qsort(FIndex, Fsize, sizeof(FIDouble), compare);
}

/**
 * Internal function called by command file, enumerates occurrences
 * 	of entries matching feature name and state abbreviation
 * Pre: feature and state are valid strings
 * 			Out is output file to write to
 * Returns: Prints results of existence search to output file
 **/
static void exists(char* feature, char* state, FILE *Out)
{
	//malloc'd: key (twice), locs
	//freed: key (twice), locs
	//char* key = malloc((strlen(feature) + 
		//strlen(state) + 3) * sizeof(char));
	char* key = calloc(501, sizeof(char));
	key = strcpy(key, feature);
	key = strcat(key, ", ");
	key = strcat(key, state);
	uint32_t* locs = StringHashTable_getLocationsOf(featIndex, key);
	free(key);
	//key = malloc((strlen(feature) + 
		//strlen(state) + 1) * sizeof(char));
	key = calloc(501, sizeof(char));
	key = strcpy(key, feature);
	key = strcat(key, " ");
	key = strcat(key, state);
	int i = 0;
	while(locs != NULL && (int)locs[i] != 0)
	{
		i++;
	}
	if (i != 0)
	{
		fprintf(Out, "%d occurrences: %s\n", i, key);
	}
	else
	{
		fprintf(Out, "Not found in index: %s\n", key);
	}
	free(locs);
	free(key);
	printLine(Out);
}

/**
 * Internal function called by command file, provides orthodromic 
 * 	distance between two locations indicated by their FIDs
 * Pre: first and second are FIDs of desired locations
 * 			Data is file containing gis records as text
 * 				Out is output file
 * Returns: Prints proper output to Out
 **/
static void distance_between(int first, int second, 
	FILE *Data, FILE *Out)
{
	//malloc'd: st (twice), insides of a,bBundle (may need to do a, b)
	//freed: st (twice), insides of a,bBundle
	FIDouble one = FIDlookup(first);
	FIDouble two = FIDlookup(second);
	
	if (one.FID != -1 && two.FID != -1)
	{
		fseek(Data, one.offset, SEEK_SET);
		char a[500];
		char b[500];
		
		fgets(a, 501, Data);
		fseek(Data, two.offset, SEEK_SET);
		
		fgets(b, 501, Data);
		StringBundle* aBundle = createStringBundle(a);
		StringBundle* bBundle = createStringBundle(b);
		
		fprintf(Out, "First:	( ");
		printGIS(Out, (*aBundle).Tokens[8], false);
		fprintf(Out, ",  ");
		printGIS(Out, (*aBundle).Tokens[7], true);
		fprintf(Out, " )  ");
		char* st = nameState(aBundle);
		fprintf(Out, "%s\n", st);
		free(st);
		
		fprintf(Out, "Second:	( ");
		printGIS(Out, (*bBundle).Tokens[8], false);
		fprintf(Out, ",  ");
		printGIS(Out, (*bBundle).Tokens[7], true);
		fprintf(Out, " )  ");
		st = nameState(bBundle);
		fprintf(Out, "%s\n", st);
		free(st);
		
		char* ptr = "\0";
		double dist = orthodrome(strtod((*aBundle).Tokens[9], &ptr), 
			strtod((*aBundle).Tokens[10], &ptr), 
				strtod((*bBundle).Tokens[9], &ptr), 
					strtod((*bBundle).Tokens[10], &ptr));
		fprintf(Out, "Distance:  %.1fkm\n", dist);
		clearStringBundle(aBundle);
		clearStringBundle(bBundle);
		free(aBundle);
		free(bBundle);
	}
	else if (one.FID == -1)
	{
		fprintf(Out, "Invalid feature ID: %d\n", first);
	}
	else
	{
		fprintf(Out, "Invalid feature ID: %d\n", second);
	}
	printLine(Out);
}

/**
 * Internal function called by command file, returns details of
 * 	all entries matching feature name, state
 * Pre: feature is string of feature name, state is string of state
 * 	abbreviation, Out is output file
 * Returns: Prints proper output of search to utput file
 **/
static void details_of(char* feature, char* state, FILE *Out, FILE *In)
{
	//malloc'd: key, locs (likely)
	//freed: key, locs
	//char* key = malloc((strlen(feature) + 
		//strlen(state) + 2) * sizeof(char));
	char* key = calloc(501, sizeof(char));
	key = strcpy(key, feature);
	key = strcat(key, ", ");
	key = strcat(key, state);
	
	uint32_t* locs = StringHashTable_getLocationsOf(featIndex, key);
	
	int i = 0;
	while (locs != NULL && (int)locs[i] != 0)
	{
		//malloc'd: insides of temp
		//freed: insides of temp
		int off = (int)locs[i];
		fseek(In, off, SEEK_SET);
		char hold[501];
		fgets(hold, 501, In);
		StringBundle* temp = createStringBundle(hold);
		
		fprintf(Out, "FID:       %s\n", (*temp).Tokens[0]);
		fprintf(Out, "Name:      %s\n", (*temp).Tokens[1]);
		fprintf(Out, "Type:      %s\n", (*temp).Tokens[2]);
		fprintf(Out, "State:     %s\n", (*temp).Tokens[3]);
		fprintf(Out, "County:    %s\n", (*temp).Tokens[5]);
		fprintf(Out, "Longitude: ");
		printGIS(Out, (*temp).Tokens[8], false);
		fprintf(Out, "   (%s)\n", (*temp).Tokens[10]);
		fprintf(Out, "Latitude:   ");
		printGIS(Out, (*temp).Tokens[7], true);
		fprintf(Out, "   (%s)\n\n", (*temp).Tokens[9]);
		
		clearStringBundle(temp);	//does temp need to be freed?
		free(temp);
		i++;
	}
	if (i == 0)
	{
		free(key);
		key = malloc((strlen(feature) + 
			strlen(state) + 1) * sizeof(char));
		key = strcpy(key, feature);
		key = strcat(key, " ");
		key = strcat(key, state);
		fprintf(Out, "Not found in index: %s\n", key);
	}
	free(locs);
	free(key);
}

/**
 * Provided hash function to be used by our hash table
 * Pre: str is proper string containing info to be hashed
 * Returns: unsigned integer number produced by hash of str
 **/
static uint32_t elfhash(const char* str)
{ 
	//Provided code, combined fields to one str with strcat and ", "
	assert(str != NULL ); 	   // self-destuct if called with NULL
	uint32_t hashvalue = 0,    // value to be return
			 high;             // high nybble of current hashvalue                 
	while ( *str )			   // continue until *str is '\0'
	{
		hashvalue = (hashvalue << 4) + *str++;     // shift high nybble out,
		                                           //   add in next char,
		                                           //   skip to the next char
        if ( (high = (hashvalue & 0xF0000000)) ) {   // if high nybble != 0000
			hashvalue = hashvalue ^ (high >> 24);  //   fold it back in
		}
		hashvalue = hashvalue & 0x7FFFFFFF;        // zero high nybble
	}
	return hashvalue;
}

/**
 * Provides local comparator function for FIDouble for use with qsort
 * Pre: a and b are valid FIDoubles
 * Returns: The difference in FIDs a from b
 **/
static int compare(const void * a, const void * b)
{
	return (*((FIDouble*)a)).FID - (*((FIDouble*)b)).FID;
}

/**
 * Reads values from command file and prints and executes accordingly
 * Pre: Out and In are proper log and command files respectively
 * Returns: Nothing, prints to log file contents of commands and their
 * 	search results
 **/
static void commandScanner(FILE *Out, FILE *In)
{
	//malloc'd: None
	//freed: None
	FILE *Data;
	char line[501] = "\0";
	_Bool instruct = false;
	int command = 1;
	while (fgets(line, 501, In) != NULL)
	{
		if (line[0] != ';' && !instruct)
		{
			char* db = strstr(line, "db_file");
			if (db != NULL)
			{
				char* emptyString = calloc(501, sizeof(char));
				sscanf(db, "db_file	%[^\n]", emptyString);
				
				Data = fopen(emptyString, "r");
				if (Data == NULL) //////////////////
				{
					printf("Data File Does Not Exist");
					fcloseall();
					exit(1);
				}
				free(emptyString);
			}
			char* ts = strstr(line, "table_sz");
			if (ts != NULL)
			{
				sscanf(line, "table_sz	%d", &tableSZ);
				//indicate setup instructions are done
				instruct = true;
				
				//Assume data index ready after reading this in
				dataIndexer(Data);
			}
		}
		else if (line[0] == ';' && instruct)
		{
			//after instruction print commments
			fprintf(Out, "%s", line);
		}
		else if (line[0] != ';' && instruct)
		{
			//Prints cmd line with proper prefix
			fprintf(Out, "Cmd  %d", command);
			fprintf(Out, ":  %s\n", line);
			
			//Takes in first field, the command to call
			//char use[501] = "\0";
			char* use = calloc(501, sizeof(char));
			use = strcpy(use, strtok(line, "\t"));
			use = realloc(use, (strlen(use) + 1) * sizeof(char));
			
			//Checks for equivalence to required commands, 
				//executes proper one
			if (strcmp(use, "exists") == 0)
			{
				char* feat = calloc(501, sizeof(char));
				feat = strcpy(feat, strtok(NULL, "\t"));
				feat = realloc(feat, (strlen(feat) + 1) * sizeof(char));
				
				char* state = calloc(501, sizeof(char));
				state = strcpy(state, strtok(NULL, "\n"));
				state = realloc(state, (strlen(state) + 1) * sizeof(char));
				exists(feat, state, Out);
				free(feat);
				free(state);
				command++;
			}
			else if (strcmp(use, "details_of") == 0)
			{
				char* feat1 = calloc(501, sizeof(char));
				feat1 = strcpy(feat1, strtok(NULL, "\t"));
				feat1 = realloc(feat1, (strlen(feat1) + 1) * sizeof(char));
				
				char* state1 = calloc(501, sizeof(char));
				state1 = strcpy(state1, strtok(NULL, "\n"));
				state1 = realloc(state1, (strlen(state1) + 1) * sizeof(char));
				details_of(feat1, state1, Out, Data);
				free(state1);
				free(feat1);
				printLine(Out);
				command++;
			}
			else if (strcmp(use, "distance_between") == 0)
			{
				int first = atoi(strtok(NULL, "\t"));
				int second = atoi(strtok(NULL, "\n"));
				distance_between(first, second, Data, Out);
				command++;
			}
			free(use);
		}
	}
	fclose(Data);
}

/**
 * Internal function producing orthodromic distance based on longitude
 * 	and latitude coordinates
 * Pre: Parameters are proper doubles indicating longitude and latitude
 * 	in degrees
 * Returns: A double of the corresponding orthodromic distance between
 * 	points
 **/
static double orthodrome(double lat1, double long1, 
	double lat2, double long2)
{
	double del = fabs(toRadii(long1) - toRadii(long2));
	double ang = acos((sin(toRadii(lat1))*sin(toRadii(lat2))) + 
		(cos(toRadii(lat1)) * cos(toRadii(lat2)) * cos(del)));
	return (ang * RADIUS);
}

/**
 * Converts degrees into radians with defined PI
 * Pre: degrees is proper double representing degrees measurement
 * Returns: degrees parameter converted to radians
 */
static double toRadii(double degrees)
{
	return (degrees * PI)/180;
}

/**
 * Performs binary search on array of FIDoubles to find indicated FID
 * Pre: FID is an integer
 * Returns: NULL if FID is not located in array, the FIDouble being
 * 	searched for
 **/
static FIDouble FIDlookup(int FID)
{
	int left = 0;
	int right = Fsize-1;
	while (left <= right)
	{
		int index = round((left + right)/2);
		FIDouble exam = FIndex[index];
		if (exam.FID == FID)
		{
			return exam;
		}
		if (exam.FID < FID)
		{
			left = index + 1;
		}
		else
		{
			right = index - 1;
		}
	}
	FIDouble null = {-1, -1};
	return null;
}

/**
 * Prints requested form of longitude/latitude coordinates
 * Pre:	Out is output file to print to
 * 			gis is string containing coordinate to convert
 * 				lat is boolean indicating whether coord is latitude 
 * 					or not, is read and printed differently
 * Returns: Nothing, prints proper form of string to Out file
 **/
void printGIS(FILE *Out, const char* const gis, _Bool lat)
{
	int d, m, s;
	char dir;
	if (!lat)
	{
		sscanf(gis, "%3d%2d%2d%c", &d, &m, &s, &dir);
		fprintf(Out, "%03dd %2dm %2ds ", d, m, s);
		printDir(Out, dir);
	}
	else
	{
		sscanf(gis, "%2d%2d%2d%c", &d, &m, &s, &dir);
		fprintf(Out, "%02dd %2dm %2ds ", d, m, s);
		printDir(Out, dir);
	}
}

/**
 * Prints lengthened version of direction character
 * Pre: Out is output file to print to
 * 			dir is character to be converted to full direction string
 * Returns: Nothing, prints direction to out file
 **/
void printDir(FILE *Out, char dir)
{
	switch (dir)
	{
		case 'N':
		{
			fprintf(Out, "North");
			break;
		}
		case 'S':
		{
			fprintf(Out, "South");
			break;
		}
		case 'W':
		{
			fprintf(Out, "West");
			break;
		}
		case 'E':
		{
			fprintf(Out, "East");
			break;
		}
	}
}

/**
 * Gives back "feature name, state" for provided StringBundle
 * Pre: record is a valid and correct StringBundle with entries in 
 * 			proper order
 * Returns: String containing record's entry as: "feature name, state"
 **/
static char* nameState(StringBundle* record)
{
	//needs to be free'd on next level
	char* key = calloc(501, sizeof(char));
	key = strcpy(key, (*record).Tokens[1]);
	key = strcat(key, ", ");
	key = strcat(key, (*record).Tokens[3]);
	key = realloc(key, (strlen(key) + 1) * sizeof(char));
	return key;
}

/**
 * Prints out a line ending in a newline character, used often in log
 * Pre: Out is valid output file for printing
 * Returns: Nothing, prints line to output file
 **/
void printLine(FILE* Out)
{
	fprintf(Out, "---------------------------------------------------\n");
}
