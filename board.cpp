// Zachary Sy

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <unistd.h>
#include <cstdlib>

#include "gfx2.h"
#include "board.h"

using namespace std;

const int height = 40;
const int width = 11;

const int visibleHeight = 21;

Board::Board(){
	// Initialize Playfield
	playfield = vector<vector<State> >(height, vector<State>(width,empty));
	fieldColors = vector<vector<Color> >(height, vector<Color>(width,Color()));
	
	RandomGenerator();
}

void Board::RandomGenerator(){
	vector<PieceStyle> pieces = {I,O,T,S,Z,J,L};	

	random_device rd;
    mt19937 g(rd());
 
	shuffle(pieces.begin(), pieces.end(), g);
	
	upcoming = pieces;
}

void Board::dispense(){
	if(upcoming.size() == 0) RandomGenerator();
	
	Tetromino piece;
	
	piece.kind = upcoming.back();
	piece.row = height/2 - 2;
	piece.col = width/2;
	
	upcoming.pop_back();
	
	placeTetromino(piece);
}

void Board::placeTetromino(Tetromino& piece){
	current = piece;
	
	switch (current.kind){
		case I:
			current.orientation = Orientation::right;
			playfield[current.row-1][current.col] = moving;
			playfield[current.row][current.col] = moving;
			playfield[current.row+1][current.col] = moving;
			playfield[current.row+2][current.col] = moving;
			break;
		case O:
			current.orientation = Orientation::up;
			playfield[current.row][current.col] = moving;
			playfield[current.row+1][current.col] = moving;
			playfield[current.row][current.col+1] = moving;
			playfield[current.row+1][current.col+1] = moving;
			break;
		case T: 
			current.orientation = Orientation::up;
			playfield[current.row][current.col] = moving;
			playfield[current.row-1][current.col] = moving;
			playfield[current.row][current.col+1] = moving;
			playfield[current.row][current.col-1] = moving;
			break;
		case S:
			current.orientation = Orientation::up;
			playfield[current.row][current.col] = moving;
			playfield[current.row][current.col-1] = moving;
			playfield[current.row-1][current.col] = moving;
			playfield[current.row-1][current.col+1] = moving;
			break;
		case Z:
			current.orientation = Orientation::up;
			playfield[current.row][current.col] = moving;
			playfield[current.row][current.col+1] = moving;
			playfield[current.row-1][current.col] = moving;
			playfield[current.row-1][current.col-1] = moving;
			break;
		case J:
			current.orientation = Orientation::left;
			playfield[current.row-1][current.col] = moving;
			playfield[current.row][current.col] = moving;
			playfield[current.row+1][current.col] = moving;
			playfield[current.row+1][current.col-1] = moving;
			break;
		case L:
			current.orientation = Orientation::right;
			playfield[current.row-1][current.col] = moving;
			playfield[current.row][current.col] = moving;
			playfield[current.row+1][current.col] = moving;
			playfield[current.row+1][current.col+1] = moving;
			break;
	}
}

Error Board::descend(){	
	Error err;
	
	int topCol = current.col-1;
	int topRow = current.row-1;
	
	current.row = current.row + 1;
	
	if(topCol < 0) topCol = 0;
	
	// Descend
	// Check if anything is in the way
	for(int row = topRow+3; row >= topRow-1; row--){
		if(row >= height) continue;
		
		for(int col = topCol+3; col >= topCol-1; col--){
			if(col <= 0) continue;

			if(playfield[row][col] == moving){
				if(row == height-1){
					err.error(blockBelow);
					return err;
				}
				
				if(playfield[row+1][col] == still){
					err.error(blockBelow);
					return err;
				}
			}
		}
	}

	// Nothing is in the way so replace blocks
	for(int row = topRow+3; row >= topRow-1; row--){
		if(row >= height) continue;
		
		for(int col = topCol+3; col >= topCol-1; col--){
			if(col <= 0) continue;

			if(playfield[row][col] == moving){
				playfield[row][col] = empty;
				playfield[row+1][col] = moving;
				
				// Colors				
				Color color = Color();
				color.background();
				fieldColors[row][col] = color;
				
				color.init(current.kind); 
				fieldColors[row+1][col] = color;
			}
		}
	}
	
	return err;
}

void Board::makeStill(){
	int topCol = current.col-1;
	int topRow = current.row-1;
	
	if(topCol < 0) topCol = 0;
	
	// Descend
	for(int row = topRow+3; row >= topRow-1; row--){
		for(int col = topCol+3; col >= topCol-1; col--){
			if(col <= 0) continue;
			if(row >= height) continue;

			if(playfield[row][col] == moving){				
				playfield[row][col] = still;
				
				Color color = Color();
				color.init(current.kind);
				
				fieldColors[row][col] = color;
			}
		}
	}
	
	dispense();
}

void Board::death(){
	for(int row = 0; row < visibleHeight; row++){
		for(State square:playfield[row]){
			if(square == still){
				// Endgame
				exit(0);
			} 
		}
	}
}

void Board::lineCleared(){
	for(int row = visibleHeight; row < height; row++){
		for(int col = 1; col < width; col++){
			if(playfield[row][col] != still){
				goto notCleared;
			} 
		}
		
		// Line Filled
		//	Delete line
		for(int col = 1; col < width; col++){
			playfield[row][col] = empty;
			fieldColors[row][col] = Color();
		}
		
		// Move every row down
		for(int aRow = row-1; aRow > visibleHeight; aRow--){
			for(int col = 1; col < width; col++){
				if(playfield[aRow][col] == still){
					playfield[aRow+1][col] = still;
					playfield[aRow][col] = empty;
					
					fieldColors[aRow+1][col] = fieldColors[aRow][col];
					fieldColors[aRow][col] = Color();
					
					
				}
			}	
		}
		
		notCleared:;
	}
}

// There is probably a better way to do this but this is due on sunday
Error Board::rotate(bool clockwise){
	Error err;
	int col = current.col;
	int row = current.row;

	if(clockwise){
		switch (current.kind){
		case I:
			switch(current.orientation){
				case up:
					// Can't rotate
					if(row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row-1][col] == still || playfield[row+1][col] == still || playfield[row+2][col] == still){
						err.error(blockBelow);
						return err;							
					}
				
					// Remove Up
					playfield[row][col+1] = empty;
					playfield[row][col-1] = empty;
					playfield[row][col-2] = empty;
					
					// Put Right
					playfield[row-1][col] = moving;
					playfield[row+1][col] = moving;
					playfield[row+2][col] = moving;

					current.orientation = Orientation::right;
					break;
					
				case down:
					// Can't rotate
					if(row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row-1][col] == still || playfield[row-2][col] == still || playfield[row+1][col] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Down
					playfield[row][col-1] = empty;
					playfield[row][col+1] = empty;
					playfield[row][col+2] = empty;
					
					// Place Left
					playfield[row-1][col] = moving;
					playfield[row-2][col] = moving;
					playfield[row+1][col] = moving;
					
					current.orientation = Orientation::left;
					break;
					
				case Orientation::left:
					// Can't rotate
					if(col+1 >= width || col-2 <= 0){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col+1] == still || playfield[row][col-1] == still || playfield[row][col-2] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Left
					playfield[row-1][col] = empty;
					playfield[row-2][col] = empty;
					playfield[row+1][col] = empty;

					// Put Up
					playfield[row][col+1] = moving;
					playfield[row][col-1] = moving;
					playfield[row][col-2] = moving;
					
					current.orientation = up;
					break;
				case Orientation::right:
					// Can't rotate
					if(col+2 >= width || col-1 <= 0){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col+1] == still || playfield[row][col-1] == still || playfield[row][col+2] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Right
					playfield[row-1][col] = empty;
					playfield[row+1][col] = empty;
					playfield[row+2][col] = empty;			
					
					// Put Down
					playfield[row][col-1] = moving;
					playfield[row][col+1] = moving;
					playfield[row][col+2] = moving;
					
					current.orientation = down;
					break;
			}
			break;
		case O:
			break;
		case T: 
			switch(current.orientation){
				case up:
					// Can't rotate
					if(row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col] == still){
						err.error(blockBelow);
						return err;							
					}
					
					playfield[row][col-1] = empty;
					playfield[row+1][col] = moving;

					current.orientation = Orientation::right;
					break;
				case down:
					// Can't rotate
					if(playfield[row-1][col] == still){
						err.error(blockBelow);
						return err;							
					}
					
					playfield[row][col+1] = empty;
					playfield[row-1][col] = moving;
					
					current.orientation = Orientation::left;
					break;
				case Orientation::left:
					// Can't rotate
					if(col+1 >= width){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					playfield[row+1][col] = empty;
					playfield[row][col+1] = moving;
					
					current.orientation = up;
					break;
				case Orientation::right:
					// Can't rotate
					if(col-1 <= 0){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col-1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					playfield[row-1][col] = empty;
					playfield[row][col-1] = moving;
					
					current.orientation = down;
					break;
			}
			break;
		case S:
			switch(current.orientation){
				case up:
					// Can't rotate
					if(col+1 >= width || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col+1] == still || playfield[row+1][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Up
					playfield[row-1][col+1] = empty;
					playfield[row][col-1] = empty;
					
					// Put Right
					playfield[row][col+1] = moving;
					playfield[row+1][col+1] = moving;

					current.orientation = Orientation::right;
					break;
					
				case down:
					// Can't rotate
					if(col-1 <= 0){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row-1][col-1] == still || playfield[row][col-1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Down
					playfield[row][col+1] = empty;
					playfield[row+1][col-1] = empty;
					
					// Place Left
					playfield[row][col-1] = moving;
					playfield[row-1][col-1] = moving;
					
					current.orientation = Orientation::left;
					break;
					
				case Orientation::left:
					// Can't rotate
					if(col-1 <= 0 || col+1 >= width){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row-1][col] == still || playfield[row-1][col+1] == still || playfield[row][col-1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Left
					playfield[row-1][col-1] = empty;
					playfield[row+1][col] = empty;

					// Put Up
					playfield[row][col-1] = moving;
					playfield[row-1][col] = moving;
					playfield[row-1][col+1] = moving;
					
					current.orientation = up;
					break;
				case Orientation::right:
					// Can't rotate
					if(col-1 <= 0 || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col] == still || playfield[row+1][col-1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Right
					playfield[row-1][col] = empty;
					playfield[row+1][col+1] = empty;
					
					// Put Down
					playfield[row+1][col] = moving;
					playfield[row+1][col-1] = moving;
					
					current.orientation = down;
					break;
			}
			
			break;
		case Z:
			switch(current.orientation){
				case up:
					// Can't rotate
					if(col-1 <= 0 || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col-1] == still || playfield[row][col-1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Up
					playfield[row-1][col-1] = empty;
					playfield[row][col+1] = empty;
					
					// Put Right
					playfield[row][col-1] = moving;
					playfield[row+1][col-1] = moving;

					current.orientation = Orientation::right;
					break;
					
				case down:
					// Can't rotate
					if(col+1 >= width){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row-1][col+1] == still || playfield[row][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Down
					playfield[row][col-1] = empty;
					playfield[row+1][col+1] = empty;
					
					// Place Left
					playfield[row][col+1] = moving;
					playfield[row-1][col+1] = moving;
					
					current.orientation = Orientation::left;
					break;
					
				case Orientation::left:
					// Can't rotate
					if(col-1 <= 0 || col+1 >= width){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row-1][col] == still || playfield[row-1][col-1] == still || playfield[row][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Left
					playfield[row-1][col+1] = empty;
					playfield[row+1][col] = empty;

					// Put Up
					playfield[row][col+1] = moving;
					playfield[row-1][col] = moving;
					playfield[row-1][col-1] = moving;
					
					current.orientation = up;
					break;
				case Orientation::right:
					// Can't rotate
					if(col+1 >= width || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col] == still || playfield[row+1][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Right
					playfield[row-1][col] = empty;
					playfield[row+1][col-1] = empty;
					
					// Put Down
					playfield[row+1][col] = moving;
					playfield[row+1][col+1] = moving;
					
					current.orientation = down;
					break;
			}
			
			break;
		case J:
			switch(current.orientation){
				case up:
					// Can't rotate
					if(col+1 >= width || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col] == still || playfield[row-1][col+1] == still || playfield[row-1][col] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Up
					playfield[row][col-1] = empty;
					playfield[row-1][col-1] = empty;
					playfield[row][col+1] = empty;
					
					// Put Right
					playfield[row-1][col+1] = moving;
					playfield[row-1][col] = moving;
					playfield[row+1][col] = moving;

					current.orientation = Orientation::right;
					break;
					
				case down:
					// Can't rotate
					if(col-1 <= 0 || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col] == still || playfield[row+1][col-1] == still || playfield[row-1][col] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Down
					playfield[row][col+1] = empty;
					playfield[row][col-1] = empty;
					playfield[row+1][col+1] = empty;
					
					// Place Left
					playfield[row-1][col] = moving;
					playfield[row+1][col-1] = moving;
					playfield[row+1][col] = moving;
					
					current.orientation = Orientation::left;
					break;
					
				case Orientation::left:
					// Can't rotate
					if(col-1 <= 0 || col+1 >= width){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col-1] == still || playfield[row+1][col-1] == still || playfield[row][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Left
					playfield[row-1][col] = empty;
					playfield[row+1][col-1] = empty;
					playfield[row+1][col] = empty;

					// Put Up
					playfield[row][col-1] = moving;
					playfield[row][col+1] = moving;
					playfield[row-1][col-1] = moving;
					
					current.orientation = up;
					break;
				case Orientation::right:
					// Can't rotate
					if(col-1 <= 0 || row+1 >= height || col+1 >= width){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col+1] == still || playfield[row+1][col+1] == still || playfield[row][col-1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Right
					playfield[row-1][col] = empty;
					playfield[row-1][col+1] = empty;
					playfield[row+1][col] = empty;
					
					// Put Down
					playfield[row][col+1] = moving;
					playfield[row][col-1] = moving;
					playfield[row+1][col+1] = moving;
					
					current.orientation = down;
					break;
			}
			
			break;
		case L:
			switch(current.orientation){
				case up:
					// Can't rotate
					if(col+1 >= width || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col] == still || playfield[row+1][col+1] == still || playfield[row-1][col] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Up
					playfield[row][col-1] = empty;
					playfield[row-1][col+1] = empty;
					playfield[row][col+1] = empty;
					
					// Put Right
					playfield[row+1][col+1] = moving;
					playfield[row-1][col] = moving;
					playfield[row+1][col] = moving;

					current.orientation = Orientation::right;
					break;
					
				case down:
					// Can't rotate
					if(col-1 <= 0 || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row+1][col] == still || playfield[row-1][col-1] == still || playfield[row-1][col] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Down
					playfield[row][col+1] = empty;
					playfield[row][col-1] = empty;
					playfield[row+1][col-1] = empty;
					
					// Place Left
					playfield[row-1][col] = moving;
					playfield[row-1][col-1] = moving;
					playfield[row+1][col] = moving;
					
					current.orientation = Orientation::left;
					break;
					
				case Orientation::left:
					// Can't rotate
					if(col+1 >= width || col-1 <= 0 || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col-1] == still || playfield[row-1][col+1] == still || playfield[row][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Left
					playfield[row-1][col] = empty;
					playfield[row-1][col-1] = empty;
					playfield[row+1][col] = empty;

					// Put Up
					playfield[row][col-1] = moving;
					playfield[row-1][col+1] = moving;
					playfield[row][col+1] = moving;
					
					current.orientation = up;
					break;
				case Orientation::right:
					// Can't rotate
					if(col+1 >= width || col-1 <= 0 || row+1 >= height){
						err.error(blockBelow);
						return err;
					}
					
					if(playfield[row][col-1] == still || playfield[row+1][col-1] == still || playfield[row][col+1] == still){
						err.error(blockBelow);
						return err;							
					}
					
					// Remove Right
					playfield[row-1][col] = empty;
					playfield[row+1][col+1] = empty;
					playfield[row+1][col] = empty;
					
					// Put Down
					playfield[row][col+1] = moving;
					playfield[row][col-1] = moving;
					playfield[row+1][col-1] = moving;
					
					current.orientation = down;
					break;
			}
			break;
	}
		
	}else{
		err = rotate();
		err = rotate();
		err = rotate();
	}
	
	return err;
}

Error Board::move(bool right){
	Error err;
	int topCol = current.col-1;
	int topRow = current.row-1;
	
	if(topCol < 0) topCol = 0;
	
	if(right){

		
		// Check if valid
		for(int row = topRow+3; row >= topRow; row--){
			if(row >= height) continue;
			
			for(int col = topCol+3; col >= topCol; col--){
				if(col <= 0) continue;
					
				if(playfield[row][col] == moving && col+1 >= width){
					err.error(hittingWall);
					return err;
				}
				
				// Hit a block
				if(playfield[row][col+1] == still){
					err.error(hittingWall);
					return err;					
				}
			}
		}
		
		// Is valid
		current.col = current.col + 1;
		
		for(int row = topRow+3; row >= topRow; row--){
			for(int col = topCol+3; col >= topCol; col--){
				if(col <= 0) continue;
					
				if(playfield[row][col] == moving){
					playfield[row][col] = empty;
					playfield[row][col+1] = moving;
					
					// Colors				
					Color color = Color();
				
					fieldColors[row][col] = color;
				
					color.init(current.kind); 
					fieldColors[row][col+1] = color;					
				}
			}
		}
		
	}else{
		// Check is valid
		for(int row = topRow; row < topRow+4; row++){
			if(row >= height) continue;
			
			for(int col = topCol; col < topCol+4; col++){
				if(col >= width) continue;
				
				if(playfield[row][col] == moving && col-1 <= 0){
					err.error(hittingWall);
					return err;
				}
				
				// Hit a block
				if(playfield[row][col-1] == still){
					err.error(hittingWall);
					return err;					
				}
			}
		}
	
		// is Valid
		current.col = current.col - 1;
		
		for(int row = topRow; row < topRow+4; row++){
			if(row >= height) continue;
			
			for(int col = topCol; col < topCol+4; col++){
				if(col >= width) continue;
			
				if(playfield[row][col] == moving){
					playfield[row][col] = empty;
					playfield[row][col-1] = moving;
					
					// Colors				
					Color color = Color();
				
					fieldColors[row][col] = color;
				
					color.init(current.kind); 
					fieldColors[row][col-1] = color;
				}
			}
		}
	}
	
	return err;
	
}
	
void Board::hardDrop(){
	Error err;
	
	while(err.success){
		err = descend();
	}
	
	makeStill();
}

void Board::terminalDisplay(bool clear){
	if(clear) system("clear");
	
	for(int i = 0; i <= 11; i++){
		cout << "-";
	}
	
	cout << endl; 
	
	for(int row = visibleHeight; row < height; row++){
		cout << "|";
		for(int col = 0; col < width; col++){
			switch(playfield[row][col]){
				case still:
					cout << "X";
					break;
				case moving:
					cout << "O";
					break;
				case empty:
					cout << ".";
					break;
			}
		}
		cout << "|" << endl;
		
	}
		
	for(int i = 0; i <= 11; i++){
		cout << "-";
	}

}


void Board::display(){
	int x = 100;
	int y = 10;
	int boxWidth = 20;
	
	// Create the border
	makeBorder(x,y,width-1,height-visibleHeight,boxWidth);
	
	// Fill the squares
	for(int row = 0; row < height-visibleHeight; row++){
		for(int col = 1; col < width; col++){
			Color color = Color();

			switch(playfield[row+visibleHeight][col]){
				case empty:
					color.clear();
					makeSquare(x+(col-1)*boxWidth, y+row*boxWidth, x+col*boxWidth, y+(row+1)*boxWidth,boxWidth);
					break;
				case still:
				case moving:
					color = fieldColors[row+visibleHeight][col];
					makeSquare(x+(col-1)*boxWidth, y+row*boxWidth, x+col*boxWidth, y+(row+1)*boxWidth, boxWidth,color);				
					break;
			}
		}
	}
	
	// Hold Box

}


// GFX figures

// Make sure x and y is less than line width
void makeBorder(int x, int y, int width, int height, int boxWidth){
	int x2 = x+width*boxWidth;
	int y2 = y+height*boxWidth;
	
	makeSquare(x,y,x2,y2,boxWidth);
}

void makeSquare(int x, int y, int x2, int y2, int boxWidth, Color fillColor){
	// Left right up down
	gfx_line(x,y,x,y2);
	gfx_line(x2,y,x2,y2);
	gfx_line(x,y,x2,y);
	gfx_line(x,y2,x2,y2);

	
	if(fillColor != Color()){
		gfx_color(fillColor.r, fillColor.g, fillColor.b);
	
		//Color Inside
		for(int i = 1; i <= x2-x; i++){
			gfx_line(x+i,y+1,x+i,y2-1);
		}
	
		fillColor.clear();
		gfx_color(fillColor.r, fillColor.g, fillColor.b);
	}
}

// Mini versions of tetrominos for hold and upcoming
// x and y are the center
void miniTetromino(int x, int y, int boxWidth, PieceStyle piece){
	switch(piece){
		case I:

			break;
		case O:

			break;
		case T: 

			break;
		case S:

			break;
		case Z:

			break;
		case J:

			break;
		case L:

			break;
	}
}