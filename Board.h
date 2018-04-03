#ifndef __BOARD_H__
#define __BOARD_H__

#include <vector>
#include <string>

struct Workspace;

// currently, b values: 0 empty, 1, off, 2, on, 3 button?!
struct Board {
	unsigned char *b;
	unsigned int w = 0;
	unsigned int h = 0;
	unsigned int lightCount = 0;
	unsigned int buttonCount = 0;
	unsigned int modulo = 2;

	bool Solved() { return lightCount == 0; }
	void Toggle( unsigned int x, unsigned int y );

	Board();
	Board(const char *boardString);
	~Board();
	Board( const Board &other );
	Board& operator=( const Board &other );
	void Clear();

	void IncreaseWidth();
	void IncreaseHeight();

	void PrintBoard();
	std::string ToString();
	void SetButton( unsigned int x, unsigned int y );
	void ToggleOne( unsigned int x, unsigned int y );
	bool FindSolution();
	void BuildBlank( bool filled = false );
	void UnToggle( unsigned int x, unsigned int y ); // used to setup puzzles
	void FromWorkspace(const Workspace &ws);

};
bool operator==( const Board &a, const Board &b );
inline bool operator!=( const Board &a, const Board &b ) { return !(a==b); }


Board LoadBoard( const char *filename, char *boardNameOut = 0 );

#endif
