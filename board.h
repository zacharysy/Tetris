// Zachary Sy

#include <vector>
#include <string>
#include "gfx2.h"

using namespace std;

// Useful Enums
enum PieceStyle{ I, O, T, S, Z, J, L };
enum Orientation{ up, down, left, right};
enum State{ empty, still, moving };

enum ErrorType{none, blockBelow, hittingWall};

enum Controls{l=81, clockwise=82,r=83,softdrop=84,harddrop=32,hold=-31};

// Useful Structs
struct Error{
	int success = true;
	ErrorType type = none;
	
	void error(ErrorType type){
		(*this).success = false;
		(*this).type = type;
	}
};

struct Tetromino{
	int row;
	int col;
	PieceStyle kind;
	Orientation orientation;
};

struct Color{
	int r = 255;
	int g = 255;
	int b = 255;
	
	void clear(){
		r = 255;
		g = 255;
		b = 255;
	}
	
	void background(){
		r = 0;
		g = 0;
		b = 0;
	}
	
	bool operator==(Color c2){
		if(c2.r == r && c2.g == g && c2.b == b){
			return true;
		}
		
		return false;
	}

	bool operator!=(Color c2){
		return !(*this==c2);
	}
	
	void init(PieceStyle style){
		switch(style){
			case I:
				// Cyan 
				r = 0;
				g = 255;
				b = 255;
				break;
			case O:
				// Yellow
				r = 255;
				g = 255;
				b = 0;				
				break;
			case T:
				// Purple
				r = 255;
				g = 0;
				b = 255;	
				break;
			case S:
				// Green
				r = 0;
				g = 255;
				b = 0;				
				break;
			case Z:
				// Red
				r = 255;
				g = 0;
				b = 0;				
				break;
			case J:
				// Purple
				r = 0;
				g = 0;
				b = 255;			
				break;
			case L:
				// Orange
				r = 255;
				g = 127;
				b = 0;	
				break;
		
		}
	}
};

// Some GFX functions
void makeBorder(int TopLeftX, int TopLeftY, int width, int height, int boxWidth);
void makeSquare(int x, int y, int x2, int y2, int boxWidth,  Color fillColor = Color());
void miniTetromino(int x, int y, int boxWidth, PieceStyle piece);
		
// Main Class
class Board{
	public:
		Board();
		void RandomGenerator();
		void dispense();
		
		void placeTetromino(Tetromino& piece);
		
		
		Error descend();
		
		void makeStill();
		void death();
		void lineCleared();
		
		// Controls
		Error rotate(bool clockwise=true);
		Error move(bool right = true);
		void hardDrop();
		
		void terminalDisplay(bool clear = true);
		void display();
		
	private:
		vector< vector<State> > playfield;
		vector< vector<Color> > fieldColors;
		vector<PieceStyle> upcoming;
		Tetromino hold;
		Tetromino current;
};