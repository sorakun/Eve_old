/*
 * Eve programming language header.
 * Eve data types.
 *
 */

#ifndef EVE_HEADER
#define EVE_HEADER

#define true  1
#define false 0
 
typedef unsigned short bool;

//Detect and define which command to use
#ifndef WIN32
	#define COMMAND "clear" //Clears a linux console screen
#else
	#define COMMAND "cls" //Clears a windows console screen
#endif

#define wipe() system( COMMAND )
//To clear the console screen use wipe();

#define wait() {printf("Hit \"Enter\" to continue\n");fflush(stdin);getchar();fflush(stdin);}
//To pause the program or hold the console screen open use wait();

#endif
