// Zachary Sy
// Tetris

#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <cstdlib>

#include "gfx2.h"
#include "board.h"

using namespace std;

void test();
void openGraphics();

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
	const int width = 700, height = 700;
	bool loop = true;
	
	Board board = Board();
	board.dispense();

	gfx_open(width,height, "Tetris!");

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
		//board.terminalDisplay();
		usleep(100000);
			
		if(gfx_event_waiting()){
			action = gfx_wait();

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
				case 'q':
					loop = false;
				default:
					break;
			}		
		
		}

  	}

}