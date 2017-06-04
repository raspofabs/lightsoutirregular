#include "util.h"

bool verbose = true;
int loggingLevel = 3;

#include "ConfigReader.h"
#include <vector>

typedef std::vector<int> ivec;
typedef std::vector<ivec> ivecvec;

struct Workspace {
	ivecvec board;
	unsigned int maxW;
};

struct Board {
	unsigned char *b;
	unsigned int w;
	unsigned int h;
	Board() : b(0) {}
	void ToggleOne( int x, int y ) {
		unsigned char &cell = b[x+y*w];
		if( cell > 0 ) {
			cell = 3 - cell;
		}
	}
	void Toggle( int x, int y ) {
		ToggleOne( x, y );	
		ToggleOne( x-1, y );	
		ToggleOne( x, y-1 );	
		ToggleOne( x+1, y );	
		ToggleOne( x, y+1 );	
	}
	bool FindSolution() {
		// count valid "buttons"
		int buttonCount = 0;

		// run through all permutations in sequence (use gray code to cover all permutations without resetting)
		unsigned int grayBits;
		for( int i = 0; i < buttonCount; ++i ) {
			grayBits = i ^ (i>>1);
			logf( 1, "Bin %-9s - Gray %-9s\n", ToBinaryString( i ), ToBinaryString( grayBits ) );
		}

		// check each time for all 1s

		// if we reach the end, then we're in an impossble situation
		return false;
	}
	void FromWorkspace(const Workspace &ws) {
		w = ws.maxW;
		h = ws.board.size();
		b = (unsigned char *)malloc( w*h );
		memset( b, 0, w * h );
		for( size_t iy = 0; iy < h; ++iy ) {
			const ivec &row = ws.board[iy];
			for( size_t ix = 0; ix < row.size(); ++ix ) {
				b[iy*w+ix] = row[ix];
			}
		}
	}
};

void AddBoardLine( const char *line, void *user ) {
	Workspace *ws = (Workspace*)user;
	ivec newRow;
	while( *line ) {
		int value = 0;
		if( *line == 'x' )
			value = 2;
		if( *line == '.' )
			value = 1;
		line += 1;
		newRow.push_back(value);
	}

	if( newRow.size() > 0 ) {
		ws->board.push_back( newRow );
		if( ws->maxW < newRow.size() )
			ws->maxW = newRow.size();
	}
}
LineReaderCallback GetCallbackFromHeader( const char *line, void * ) {
	if( 0 == strcasecmp( line, "board" ) ) { return AddBoardLine; }
	logf( 1, "Unexpected config header [%s], returning null handler\n", line );
	return 0;
}


#define TestAssert( X ) if( !( X ) ) { logf( 1, RED "TEST FAILED" CLEAR "[%s]\n", #X ); return false; } else { logf( 3, GREEN "TEST PASSED" CLEAR "[%s]\n", #X ); }

bool RunPuzzle( int testID ) {
	char filename[128];
	sprintf( filename, "test%i.txt", testID );
	Workspace ws;
	int result = OpenConfigAndCallbackPerLine( filename, GetCallbackFromHeader, 0, &ws );
	TestAssert( 0 == result );

	Board b; b.FromWorkspace( ws );

	TestAssert( b.FindSolution() );

	return true;
}

const int MAX_PUZZLE_ID = 4;

int main( ) {
	for( int pid = 0; pid <= MAX_PUZZLE_ID; ++pid ) {
		RunPuzzle(pid);
	}

	//logf( 1, GREEN "ALL PUZZLES SOLVED" CLEAR "\n" );

	return 0;
}
