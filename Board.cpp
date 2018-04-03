#include "Board.h"
#include "util.h"
#include "ConfigReader.h"
#include <assert.h>

typedef std::vector<int> ivec;
typedef std::vector<ivec> ivecvec;

struct Workspace {
	ivecvec board;
	unsigned int maxW = 0;
	std::string boardName;
	unsigned int modulo = 2;
};


Board::Board() : b(0) {
}
Board& Board::operator=( const Board &other ) {
	Clear();
	w = other.w;
	h = other.h;
	modulo = other.modulo;
	b = (unsigned char *)malloc( w*h );
	lightCount = other.lightCount;
	buttonCount = other.buttonCount;
	memcpy( b, other.b, w * h );
	return *this;
}
Board::Board( const Board &other ) {
	w = other.w;
	h = other.h;
	assert( w > 0 );
	assert( h > 0 );
	modulo = other.modulo;
	b = (unsigned char *)malloc( w*h );
	lightCount = other.lightCount;
	buttonCount = other.buttonCount;
	memcpy( b, other.b, w * h );
}
Board::~Board() {
	Clear();
}
void Board::Clear() {
	if( b ) {
		free( b );
		b = 0;
	}
}
void Board::ClearContent() {
	for( unsigned int y = 0; y < h; ++y ) {
		for( unsigned int x = 0; x < w; ++x ) {
			int cellIndex = x + y * w;
			char cellVal = b[cellIndex];
			if( cellVal ) {
				b[cellIndex] = 1;
			}
		}
	}
}
void Board::BuildBlank( bool filled ) {
	b = (unsigned char *)malloc( w*h );
	lightCount = 0;
	buttonCount = 0;
	if( filled ) {
		memset( b, 1, w * h );
	} else {
		memset( b, 0, w * h );
	}
}
void Board::FromWorkspace(const Workspace &ws) {
	w = ws.maxW;
	h = ws.board.size();
	modulo = ws.modulo;
	BuildBlank();
	for( size_t iy = 0; iy < h; ++iy ) {
		const ivec &row = ws.board[iy];
		for( size_t ix = 0; ix < row.size(); ++ix ) {
			b[iy*w+ix] = row[ix];
			if( row[ix] >= 2 )
				lightCount += 1;
			if( row[ix] != 0 )
				buttonCount += 1;
		}
	}
}

void Board::IncreaseWidth() {
	w+=1;
	b = (unsigned char*)realloc( b, w*h );
	for( unsigned int y = h-1; y < h; --y ) {
		for( unsigned int x = w-1; x < w; --x ) {
			if( x == w-1 ) {
				b[x+y*w] = 0;
			} else {
				b[x+y*w] = b[x+y*(w-1)];
			}
		}
	}
}
void Board::IncreaseHeight() {
	h+=1;
	b = (unsigned char*)realloc( b, w*h );
	for( unsigned int x = 0; x < w; ++x ) {
		b[x+(h-1)*w] = 0;
	}
}
void Board::PrintBoard() {
	logf( 1, " Lights %i - %ix%i\n", lightCount, w, h );
	for( unsigned int y = 0; y < h; ++y ) {
		char buff[32] = {0};
		char pb[] = " .xO";
		for( unsigned int x = 0; x < w; ++x ) {
			buff[x] = pb[b[y*w+x]];
		}
		logf( 1, "> %s\n", buff );
	}
}
std::string Board::ToString() {
	std::string buffer;
	for( unsigned int y = 0; y < h; ++y ) {
		char buff[32] = {0};
		char pb[] = " .x23456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		for( unsigned int x = 0; x < w; ++x ) {
			buff[x] = pb[b[y*w+x]];
		}
		buffer += buff;
		buffer += "\n";
	}
	return buffer;
}
void Board::ToggleOne( unsigned int x, unsigned int y ) {
	if( x >= w || y >= h )
		return;
	unsigned char &cell = b[x+y*w];
	if( cell > 0 ) {
		if( cell == 1 ) {
			++lightCount;
		}
		cell += 1;
		if( cell > modulo )
			cell -= modulo;
		if( cell == 1 ) {
			--lightCount;
		}
	}
}
void Board::Toggle( unsigned int x, unsigned int y ) {
	ToggleOne( x, y );	
	ToggleOne( x-1, y );	
	ToggleOne( x, y-1 );	
	ToggleOne( x+1, y );	
	ToggleOne( x, y+1 );	
}
void Board::UnToggle( unsigned int x, unsigned int y ) {
	for( unsigned int i = 1; i < modulo; ++i ) {
		ToggleOne( x, y );	
		ToggleOne( x-1, y );	
		ToggleOne( x, y-1 );	
		ToggleOne( x+1, y );	
		ToggleOne( x, y+1 );	
	}
}
bool Board::IsOn( unsigned int x, unsigned int y ) {
	if( x >= w || y >= h )
		return false;
	unsigned char &cell = b[x+y*w];
	if( cell > 0 ) {
		if( cell == 1 ) {
			return false;
		}
		return true;
	}
	return false;
}
bool Board::FindEliminationSolution() {
	//PrintBoard();
	// count valid "buttons"
	static const int MAX_BUTTONS = 64;

	if( buttonCount > MAX_BUTTONS ) {
		logf( 2, "%i buttons is too many for eliminination too.\n", buttonCount );
		return false;
	}

	bool goal[MAX_BUTTONS] = { false };
	unsigned char btnx[MAX_BUTTONS];
	unsigned char btny[MAX_BUTTONS];
	{
		unsigned int btn = 0;
		for( size_t y = 0; y < h; ++y ) {
			for( size_t x = 0; x < w; ++x ) {
				if( b[x+y*w] != 0 ) {
					goal[btn] = IsOn( x, y );
					if( btn < MAX_BUTTONS ) {
						btnx[btn] = x;
						btny[btn] = y;
					}
					btn += 1;
				}
			}
		}
		assert( btn == buttonCount );
	}

	// build as many boards as we have buttons
	Board blank = *this;
	blank.ClearContent();

	static const unsigned int ROWLEN = MAX_BUTTONS*2;
	bool rows[MAX_BUTTONS*ROWLEN]; memset( rows, 0, sizeof( rows ) );
	bool temp[ROWLEN]; memset( temp, 0, sizeof( temp ) );

	for( unsigned int btn = 0; btn < buttonCount; ++btn ) {
		bool *row = &rows[ROWLEN*btn];
		blank.Toggle( btnx[btn], btny[btn] );
		for( unsigned int btn = 0; btn < buttonCount; ++btn ) {
			row[btn] = blank.IsOn( btnx[btn], btny[btn] );
		}
		row[btn + buttonCount] = true;
		blank.Toggle( btnx[btn], btny[btn] );
	}

	printf( "Matrix before elimination\n" );
	for( unsigned int row = 0; row < buttonCount; ++row ) {
		for( unsigned int column = 0; column < buttonCount; ++column ) {
			printf( "%c", ".#"[rows[ROWLEN*row+column]] );
		}
		printf( " " );
		for( unsigned int column = 0; column < buttonCount; ++column ) {
			printf( "%c", ".#"[rows[ROWLEN*row+column+buttonCount]] );
		}
		printf( "\n" );
	}

	// do the elimination
	unsigned int firstLegitimateRow = 0;
	for( unsigned int column = 0; column < buttonCount-1; ++column ) {
		// find the top row with this column active
		unsigned int pivot = MAX_BUTTONS;
		for( unsigned int r = firstLegitimateRow; r < buttonCount; ++r ) {
			if( rows[ROWLEN*r + column] ) {
				pivot = r;
				//logf( 1, "Column %i pivot in row %i\n", column, pivot );
				break;
			}
		}

		// legitimate pivot
		if( pivot < MAX_BUTTONS ) {
			// swap pivot up to first row
			memcpy( temp, &rows[ROWLEN*pivot], sizeof(temp) );
			if( pivot != firstLegitimateRow ) {
				//logf( 1, "Column %i swap rows %i and %i\n", column, pivot, firstLegitimateRow );
				memcpy( &rows[ROWLEN*pivot], &rows[ROWLEN*firstLegitimateRow], sizeof(temp) );
				memcpy( &rows[ROWLEN*firstLegitimateRow], temp, sizeof(temp) );
				pivot = firstLegitimateRow;
			}
			firstLegitimateRow+=1;
			for( unsigned int r = firstLegitimateRow; r < buttonCount; ++r ) {
				// if we have a match, then toggle all matching
				if( rows[ROWLEN*r + column] ) {
					//logf( 1, "Row %i needs combining with pivot\n", r );
					for( unsigned int c = column; c < buttonCount*2; ++c ) {
						if( temp[c] ) {
							//logf( 1, "Row %i toggle element %i\n", r, c );
							rows[ROWLEN*r + c] ^= 1;
						}
					}
				}
			}
		} else {
			logf( 1, "No legitimate pivot for column %i\n", column );
		}
	}

	// now cleanup the rows, eliminating what can be eliminated
	for( unsigned int column = firstLegitimateRow; column > 0; --column ) {
		// we're working bottom up,
		// toggle all rows with the current pivot if they have that pivot column
		memcpy( temp, &rows[ROWLEN*column], sizeof(temp) );

		for( unsigned int r = 0; r < column; ++r ) {
			// if we have a match, then toggle all matching
			if( rows[ROWLEN*r + column] ) {
				//logf( 1, "Row %i needs combining with pivot\n", r );
				for( unsigned int c = column; c < buttonCount*2; ++c ) {
					if( temp[c] ) {
						//logf( 1, "Row %i toggle element %i\n", r, c );
						rows[ROWLEN*r + c] ^= 1;
					}
				}
			}
		}
	}

	printf( "Matrix after elimination\n" );
	for( unsigned int row = 0; row < buttonCount; ++row ) {
		for( unsigned int column = 0; column < buttonCount; ++column ) {
			printf( "%c", ".#"[rows[ROWLEN*row+column]] );
		}
		printf( " " );
		for( unsigned int column = 0; column < buttonCount; ++column ) {
			printf( "%c", ".#"[rows[ROWLEN*row+column+buttonCount]] );
		}
		printf( "\n" );
	}
	printf( "\n" );
	// using the matrix generated, clear the problem board
	logf( 1, "Use the created matrix\n" );
	memset( temp, 0, sizeof(temp) );
	for( unsigned int column = 0; column < buttonCount; ++column ) {
		// we're building up the button presses
		// toggle temp rows with the current row if it matches the goal
		if( goal[column] ) {
			for( unsigned int c = column; c < buttonCount*2; ++c ) {
				if( rows[ROWLEN*column + c] ) {
					temp[c] ^= 1;
				}
			}
		}
	}

	for( unsigned int column = 0; column < buttonCount; ++column ) {
		printf( "%c", ".#"[temp[column]] );
	}
	printf( " " );
	for( unsigned int column = 0; column < buttonCount; ++column ) {
		printf( "%c", ".#"[temp[column+buttonCount]] );
	}
	printf( "\n" );

	return true;
	
	// if we reach the end, then we're in an impossble situation
	return false;
}
bool Board::FindSolution() {
	if( modulo != 2 ) {
		logf( 1, "No solver for modulo greater than 2 boards\n" );
		return false;
	}
	//PrintBoard();
	// count valid "buttons"
	static const int MAX_BUTTONS = 8;

	unsigned int permutations = 1U<<buttonCount;
	if( buttonCount > MAX_BUTTONS ) {
		logf( 2, "Run through %i buttons = %i permutations, so not doing it brute force\n", buttonCount, permutations );
		return FindEliminationSolution();
	}

	unsigned char btnx[MAX_BUTTONS];
	unsigned char btny[MAX_BUTTONS];
	{
		unsigned int btn = 0;
		for( size_t y = 0; y < h; ++y ) {
			for( size_t x = 0; x < w; ++x ) {
				if( b[x+y*w] != 0 ) {
					if( btn < MAX_BUTTONS ) {
						btnx[btn] = x;
						btny[btn] = y;
					}
					btn += 1;
				}
			}
		}
		assert( btn == buttonCount );
	}
	
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


static void AddBoardName( const char *line, void *user ) {
	Workspace *ws = (Workspace*)user;
	ws->boardName = line;
}
static void AddBoardModulo( const char *line, void *user ) {
	Workspace *ws = (Workspace*)user;
	ws->modulo = atoi( line );
}
static void AddBoardLine( const char *line, void *user ) {
	Workspace *ws = (Workspace*)user;
	ivec newRow;
	while( *line ) {
		int value = 0;
		if( *line == 'x' )
			value = 2;
		if( *line == '.' )
			value = 1;
		if( *line >= '2' && *line <= '9' )
			value = (*line-'2') + 3;
		line += 1;
		newRow.push_back(value);
	}

	if( newRow.size() > 0 ) {
		logf(4, "New row, old size %i -> %i\n", ws->maxW, newRow.size() );
		ws->board.push_back( newRow );
		if( ws->maxW < newRow.size() )
			ws->maxW = newRow.size();
	}
}
static LineReaderCallback GetCallbackFromHeader( const char *line, void * ) {
	if( 0 == strcasecmp( line, "board" ) ) { return AddBoardLine; }
	if( 0 == strcasecmp( line, "modulo" ) ) { return AddBoardModulo; }
	if( 0 == strcasecmp( line, "name" ) ) { return AddBoardName; }
	logf( 1, "Unexpected config header [%s], returning null handler\n", line );
	return 0;
}

Board LoadBoard( const char *filename, char *boardNameOut ) {
	Board b;

	Workspace ws;
	int result = OpenConfigAndCallbackPerLine( filename, GetCallbackFromHeader, 0, &ws );
	assert( result == 0 );
	if( result == 0 ) {
		b.FromWorkspace( ws );
		Board c = b;
		//assert( c.FindSolution() );
		if( boardNameOut && ws.boardName.size() ) {
			strcpy( boardNameOut, ws.boardName.c_str() );
		}
	}

	return b;
}

typedef std::vector<std::string> c_strings;
c_strings SplitStringOn( const char *inString, const char *splitChars = "\n\r" ) {
	char *tstr = strdup( inString );
	char *currentSplit = strtok( tstr, splitChars );

	c_strings out;
	while( currentSplit ) {
		out.push_back( currentSplit );
		currentSplit = strtok( 0, splitChars );
	}
	free( tstr );
	return out;
}

Board::Board( const char *boardString ) {
	c_strings lines = SplitStringOn( boardString );
	Workspace ws;
	for( auto line : lines ) {
		AddBoardLine( line.c_str(), &ws );
	}
	FromWorkspace( ws );
}

bool operator==( const Board &a, const Board &b ) {
	if( a.w != b.w )
		return false;
	if( a.h != b.h )
		return false;
	if( a.modulo != b.modulo )
		return false;
	for( int c = 0; c < (int)( a.w * a.h ); ++c ) {
		if( a.b[c] != b.b[c] )
			return false;
	}
	return true;
}
