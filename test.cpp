#include "util.h"

bool verbose = true;
int loggingLevel = 3;

#include "ConfigReader.h"
#include <vector>

#define TestAssert( X ) if( !( X ) ) { logf( 1, RED "TEST FAILED" CLEAR "[%s]\n", #X ); return false; } else { logf( 3, GREEN "TEST PASSED" CLEAR "[%s]\n", #X ); }

#include "Board.h"

bool RunPuzzle( int testID ) {
	char filename[128];
	sprintf( filename, "test%i.txt", testID );

	Board b = LoadBoard( filename );

	TestAssert( b.FindSolution() );

	return true;
}

const int MAX_PUZZLE_ID = 9;

int main( ) {
	for( int pid = 0; pid <= MAX_PUZZLE_ID; ++pid ) {
		RunPuzzle(pid);
	}

	//logf( 1, GREEN "ALL PUZZLES SOLVED" CLEAR "\n" );

	return 0;
}
