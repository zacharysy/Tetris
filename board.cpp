// Zachary Sy

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <sstream>
#include <unistd.h>
#include <cstdlib>

#include "gfx2.h"
#include "board.h"

using namespace std;

const int height = 40;
const int width = 11;

const int visibleHeight = 21;

const int upcomingSize = 4;

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
	
	pieces.insert(pieces.end(), upcoming.begin(), upcoming.end());
	upcoming = pieces;
}

void Board::dispense(){
	if(upcoming.size() == upcomingSize+1) RandomGenerator();
	
	Tetromino piece;
	
	piece.kind = upcoming.back();
	piece.row = height/2 - 2;
	piece.col = width/2;
	
	upcoming.pop_back();
	
	placeTetromino(piece);
	
	// Reset Hold
	if(isHeld){
		isHeld = false;
	}
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
		default:
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
	for(int row = topRow+4; row >= topRow-4; row--){
		if(row >= height) continue;
		
		for(int col = topCol+4; col >= topCol-4; col--){
			if(col <= 0 || col >= width) continue;

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
	for(int row = topRow+4; row >= topRow-4; row--){
		if(row >= height) continue;
		
		for(int col = topCol+4; col >= topCol-4; col--){
			if(col <= 0 || col >= width) continue;

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
	for(int row = topRow+4; row >= topRow-4; row--){
		for(int col = topCol+4; col >= topCol-4; col--){
			if(col <= 0 || col >= width) continue;
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
	for(int row = 0; row < visibleHeight-1; row++){
		for(State square:playfield[row]){
			if(square == still){
				// Endgame
				gfx_text(20, 20, "LOST! PRESS ANY KEY TO EXIT");
				bool a = true;
				
				while(a){
					if(gfx_event_waiting()) a = !a;
				}
				
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
		linesCleared += 1;
		
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
	
	// Level Up every 5 lines
	level = linesCleared/5 + 1;
}

int Board::getLinesCleared(){
	return linesCleared;
}

int Board::getLevel(){
	return level;
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
		default:
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
		for(int row = topRow+4; row >= topRow-4; row--){
			if(row >= height) continue;
			
			for(int col = topCol+4; col >= topCol-4; col--){
				if(col <= 0 || col >= width) continue;
					
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
		
		for(int row = topRow+4; row >= topRow-4; row--){
			for(int col = topCol+4; col >= topCol-4; col--){
				if(col <= 0 || col >= width) continue;
					
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
		for(int row = topRow-4; row < topRow+4; row++){
			if(row >= height) continue;
			
			for(int col = topCol-4; col < topCol+4; col++){
				if(col >= width || col <= 0) continue;
				
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
		
		for(int row = topRow-4; row < topRow+4; row++){
			if(row >= height) continue;
			
			for(int col = topCol-4; col < topCol+4; col++){
				if(col >= width || col <= 0) continue;
			
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

void Board::hold(){
	if(!isHeld){
		bool goDispense = false;
		
		if(holdPiece.kind == NONE){
			goDispense = true;	
			holdPiece = current;
		}else{	
			holdPiece.row = height/2 - 2;
			holdPiece.col = width/2;
			
			Tetromino temp = current;
			
			placeTetromino(holdPiece);
			
			holdPiece = temp;
		}
		
		
		// Remove moving blocks	
		for(int row = visibleHeight; row < height; row++){
			for(int col = 0; col < width; col++){
				if(playfield[row][col] == moving){
					playfield[row][col] = empty;			
				}		
			}
		}
		
		if(goDispense){
			dispense();
		}
		
		isHeld = true;
	}
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
	int x = 90;
	int y = 40;
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
					makeSquare(x+(col-1)*boxWidth, y+row*boxWidth, boxWidth, boxWidth);
					break;
				case still:
				case moving:
					color = fieldColors[row+visibleHeight][col];
					makeSquare(x+(col-1)*boxWidth, y+row*boxWidth, boxWidth, boxWidth,color);				
					break;
			}
		}
	}
	
	// Hold Box
	int holdMargin = 15;
	int holdBox = boxWidth*2;
	
	int holdx = x-holdMargin-holdBox;
	int holdy = y + 20;
	
	gfx_text(holdx, holdy, "HOLD");
	gfx_rectangle(holdx, holdy+5, holdBox, holdBox);
	
	if(holdPiece.kind != NONE){
		miniTetromino(holdx+holdBox/2, holdy+5+holdBox/2, holdBox, holdPiece.kind);
	}
	
	// Upcoming 
	int upcomingx = x + width*boxWidth;
	int upcomingy = holdy;
	
	gfx_text(upcomingx, upcomingy, "UPCOMING");
	
	for(int i = 0; i <= upcomingSize; i++){
		PieceStyle nextPiece = upcoming[upcoming.size()-1-i];
		
		gfx_rectangle(upcomingx, upcomingy+i*(holdBox+10)+10, holdBox, holdBox);
		miniTetromino(upcomingx+holdBox/2, upcomingy+i*(holdBox+10)+10+holdBox/2, holdBox, nextPiece);
	}
	
	// Level
	int levely = holdy + holdBox + 50;
	
	ostringstream levelText;
	levelText << "Level: " << level;
	
	gfx_text(holdx, levely, levelText.str().c_str());
	
	// Lines Cleared
	ostringstream linesText;
	linesText << "Lines: " << linesCleared;

	gfx_text(holdx, levely+20, linesText.str().c_str());

	

}


// GFX figures

// Make sure x and y is less than line width
void makeBorder(int x, int y, int width, int height, int boxWidth){
	makeSquare(x,y,width*boxWidth,height*boxWidth);
}

void makeSquare(int x, int y, int width, int height, Color fillColor){
	gfx_rectangle(x,y,width,height);
	
	if(fillColor != Color()){
		gfx_color(fillColor.r, fillColor.g, fillColor.b);
		gfx_fill_rectangle(x+1,y+1,width-2,height-2);
		fillColor.clear();
		gfx_color(fillColor.r, fillColor.g, fillColor.b);
	}
}

// Mini versions of tetrominos for hold and upcoming
// x and y are the center
void miniTetromino(int x, int y, int boxWidth, PieceStyle piece){
	Color color = Color();
	color.init(piece);
	
	gfx_color(color.r,color.g,color.b);
	
	switch(piece){
		case I:
			gfx_fill_rectangle(x,y-boxWidth*0.35, 4, boxWidth*0.7);
			break;
		case O:
			gfx_fill_rectangle(x-boxWidth*0.2,y-boxWidth*0.2, boxWidth*0.5, boxWidth*0.5);
			break;
		case T: 
			gfx_fill_rectangle(x,y-boxWidth*0.2, 4, boxWidth*0.3);
			gfx_fill_rectangle(x-boxWidth*0.2,y,boxWidth*0.5, 4);
			break;
		case S:
			gfx_fill_rectangle(x-boxWidth*0.1,y+4,boxWidth*0.25, 4);
			gfx_fill_rectangle(x,y,boxWidth*0.25, 4);
			break;
		case Z:
			gfx_fill_rectangle(x-boxWidth*0.1,y,boxWidth*0.25, 4);
			gfx_fill_rectangle(x,y+4,boxWidth*0.25, 4);
			break;
		case J:
			gfx_fill_rectangle(x,y-boxWidth*0.3, 4, boxWidth*0.3);
			gfx_fill_rectangle(x-boxWidth*0.2,y,boxWidth*0.3, 4);
			break;
		case L:
			gfx_fill_rectangle(x-boxWidth*0.2,y-boxWidth*0.3, 4, boxWidth*0.3);
			gfx_fill_rectangle(x-boxWidth*0.2,y,boxWidth*0.3, 4);
			break;
		default:
			break;
	}
	
	color.clear();
	gfx_color(color.r,color.g,color.b);
}