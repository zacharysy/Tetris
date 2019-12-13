// Zachary Sy
// Tetris

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <unistd.h>
#include <cstdlib>

#include "gfx2.h"
#include "board.h"

using namespace std;

void test();
void openGraphics();
void controls(Board& board, char action, bool& loop);

int main(int argc, char *argv[]){	
	//test();
	openGraphics();


	return 0;
}

void test(){
	Board board = Board();
	
	board.dispense();
	
	while(true){
		board.move(false);
		Error err = board.descend();
		
		if(!err.success){
			board.makeStill();
		}
		
		board.lineCleared();
		board.death();

		
		board.terminalDisplay();
		usleep(100000);	
	}


}

void openGraphics(){
	int action;
	const int width = 400, height = 500;
	bool loop = true;
	
	float level;
	int time;
	
	gfx_open(width,height, "Tetris!");
	
			
	Board board = Board();
	board.dispense();

	while (loop) {	
		Error err = board.descend();
		
		if(!err.success){
			board.makeStill();
		}
		
		board.lineCleared();
		board.death();
		
		gfx_clear();
		board.display();
		gfx_flush();
		
		level = board.getLevel();
		time =  200000*pow(0.8,level);
		
		usleep(time);
			
		if(gfx_event_waiting()){
			action = gfx_wait();
			controls(board, action, loop);
		
		}

  	}
}

void controls(Board& board, char action, bool& loop){
	switch (action) {
		case Controls::l:
			board.move(false);
			break;
		case Controls::r:
			board.move();
			break;
		case Controls::clockwise:
			board.rotate();
			break;
		case Controls::softdrop:
			board.descend();
			break;
		case Controls::harddrop:
			board.hardDrop();
			break;
		case Controls::hold:
			board.hold();
			break;
		case 'q':
			loop = false;
		default:
			break;
	}

}