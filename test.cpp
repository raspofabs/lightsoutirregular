#include "util.h"

bool verbose = true;
int loggingLevel = 3;

#include "ConfigReader.h"
#include <vector>

#define TestAssert( X ) if( !( X ) ) { logf( 1, RED "TEST FAILED" CLEAR "[%s]\n", #X ); return false; } else { logf( 3, GREEN "TEST PASSED" CLEAR "[%s]\n", #X ); }


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
	unsigned int lightCount;
	unsigned int buttonCount;
	Board() : b(0) {}
	void PrintBoard() {
		logf( 1, " Lights %i\n", lightCount );
		for( unsigned int y = 0; y < h; ++y ) {
			char buff[32] = {0};
			char pb[] = " .x";
			for( unsigned int x = 0; x < w; ++x ) {
				buff[x] = pb[b[y*w+x]];
			}
			logf( 1, "> %s\n", buff );
		}
	}
	void ToggleOne( unsigned int x, unsigned int y ) {
		if( x >= w || y >= h )
			return;
		unsigned char &cell = b[x+y*w];
		if( cell > 0 ) {
			cell = 3 - cell;
			lightCount += cell*2-3;
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
		PrintBoard();
		// count valid "buttons"
		unsigned char btnx[32];
		unsigned char btny[32];
		{
			unsigned int btn = 0;
			for( size_t y = 0; y < h; ++y ) {
				for( size_t x = 0; x < w; ++x ) {
					if( b[x+y*w] != 0 ) {
						btnx[btn] = x;
						btny[btn] = y;
						btn += 1;
					}
				}
			}
		}


		unsigned int permutations = 1U<<buttonCount;
		logf( 2, "Run through %i buttons = %i permutations\n", buttonCount, permutations );
		// run through all permutations in sequence (use gray code to cover all permutations without resetting)
		unsigned int grayBits = 0;
		for( unsigned int i = 0; i < permutations; ++i ) {
			unsigned int changeBit = grayBits;
			grayBits = i ^ (i>>1);
			changeBit ^= grayBits;
			changeBit = hibit( changeBit );
			//logf( 1, "Bin %-9s - Gray %-9s - Change %i\n", ToBinaryString( i ), ToBinaryString( grayBits ), changeBit );
			if( changeBit > 0 ) {
				//logf( 1, "Toggle %i,%i\n", btnx[changeBit-1], btny[changeBit-1] );
				Toggle( btnx[changeBit-1], btny[changeBit-1] );
			}
			//PrintBoard();

			// check each time for all 1s
			if( lightCount == 0 ) {
				// Solution
				for( unsigned int i = 0; i < 32; ++i ) {
					if( (1U<<i) & grayBits ) {
						logf( 1, "Toggle %i,%i\n", btnx[i], btny[i] );
					}
				}
				return true;
			}
		}


		// if we reach the end, then we're in an impossble situation
		return false;
	}
	void FromWorkspace(const Workspace &ws) {
		w = ws.maxW;
		h = ws.board.size();
		b = (unsigned char *)malloc( w*h );
		lightCount = 0;
		buttonCount = 0;
		memset( b, 0, w * h );
		for( size_t iy = 0; iy < h; ++iy ) {
			const ivec &row = ws.board[iy];
			for( size_t ix = 0; ix < row.size(); ++ix ) {
				b[iy*w+ix] = row[ix];
				if( row[ix] == 2 )
					lightCount += 1;
				if( row[ix] != 0 )
					buttonCount += 1;
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

const int MAX_PUZZLE_ID = 5;

int main( ) {
	for( int pid = 0; pid <= MAX_PUZZLE_ID; ++pid ) {
		RunPuzzle(pid);
	}

	//logf( 1, GREEN "ALL PUZZLES SOLVED" CLEAR "\n" );

	return 0;
}
