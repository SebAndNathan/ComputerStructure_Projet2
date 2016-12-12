#include<stddef.h>
#include<stdbool.h>
#include "gridHandler.h"

bool equalPos(Position p1, Position p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

static bool* ptrInGrid(Grid grid, Position position) {
	if (!(0 <= position.x && position.x < grid.width && 0 <= position.y && position.y < grid.height)) {
		fprintf(stderr, "ptrInGrid accessed out of the grid");
		return NULL;
	}
	return &(grid.storage[position.x * grid.width + position.y]);
}

bool getInGrid(Grid grid, Position position) {
	bool* ptr = ptrInGrid(grid, position);
	return ptr == NULL ? obstacle : *ptr;
}
 
void setInGrid(Grid grid, Position position, bool value) {
   bool* ptr = ptrInGrid(grid, position);
	if (ptr != NULL) {
		*ptr = value;
	}
}

void fillGridRandomly(Grid* grid) {
    for (Position p = {0, 0}; p.y < grid->height; p.y++) {
        for (p.x = 0; p.x < grid->width; p.x++) {
            setInGrid(*grid, p, (rand()%3) == 0 ? obstacle : !obstacle); // one position in 3 is an obstacle
        }
    }
    
    // borders are set to be obstacles
    for (int y = 0; y < grid->height; y++) {
        Position left = {0, y}, right = {grid->width - 1, y};
        setInGrid(*grid, left, obstacle);
        setInGrid(*grid, right, obstacle);
    }
    for (int x = 0; x < grid->width; x++) {
        Position down = {x, 0}, up = {x, grid->height - 1};
        setInGrid(*grid, down, obstacle);
        setInGrid(*grid, up, obstacle);
    }
    
    Position start, finish;
    start.x = (rand() % (grid->width - 2)) + 1;
    start.y = (rand() % (grid->height - 2)) + 1;
    
    setInGrid(*grid, start, !obstacle);
    grid->start = start;
    
    do {
        finish.x = (rand() % (grid->width - 2)) + 1;
        finish.y = (rand() % (grid->height - 2)) + 1;
    } while (equalPos(start, finish))
        
    setInGrid(*grid, finish, !obstacle);
    grid->finish = finish;
}

//  0 fine
// -1 read failed
// -2 out of bounds
static int readPosition(FILE* gridFile, Grid grid, Position* p) {
    if(fscanf(gridFile, "%d %d", &(p->x), &(p->y)) == -1) {
        return -1;
    } else if (p->x >= (grid.width-2) || p->x <= 0 || p->y >= (grid.height-2) || p->y <= 0) {
        fprintf(stderr, "position in the file is out of bound \n");
        return -2;
    } else {
       	p->y = grid.height - 1 - p->y;
        return 0;
    }
}

int fillgridWithFile(Grid* grid, char* name) {
	FILE* gridFile = fopen(path, "r");
    int result = -1;
    if (gridFile != NULL) {
    	Position tmpPos;
        
        // fill with free tiles
        for (Position p = {0, 0}; p.y < grid->height; p.y++) {
            for (p.x = 0; p.x < grid->width; p.x++) {
                setInGrid(*grid, p, !obstacle);
            }
        }
    	//begin tile
	    if(readPosition(gridFile, *grid, &tmpPosition) == 0) {
            grid->start = tmpPos;
            // end tile
	        if(readPosition(gridFile, *grid, &tmpPosition) == 0) {
                grid->finish = tmpPos;
        
                int readStatus;
                
                while ((readStatus = readPosition(gridFile, *grid, &tmpPosition)) == 0) {
                    setInGrid(*grid, tmpPos, obstacle);
                }
                // it must be because read failed
                if (readStatus == -1) {
                    bool missingBorder = false;
                    // check borders are obstacles
                    for (int y = 0; !missingBorder && y < grid->height; y++) {
                        Position left = {0, y}, right = {grid->width - 1, y};
                        missingBorder = getInGrid(*grid, left) != obstacle || getInGrid(*grid, right) != obstacle;
                    }
                    for (int x = 0; !missingBorder && x < grid->width; x++) {
                        Position down = {x, 0}, up = {x, grid->height - 1};
                        missingBorder = getInGrid(*grid, down) != obstacle || getInGrid(*grid, up) != obstacle;
                    }
                    if (!missingBorder) {
                    	// checks if there is a tile on the stating/ending position
                    	if(getInGrid(*grid, grid->start) != obstacle && getInGrid(*grid, grid->finish) != obstacle) {
                            result = 0;
                        } else {
                            fprintf(stderr, "there is a tile over the begin/end position");
                    	}
                    } else {
                        fprintf(stderr, "missing border");
                    }
                } else {
                    fprintf(stderr, "bad obstacle position \n");
                }
            } else {
                fprintf(stderr, "bad finish position \n");
            }
        } else {
            fprintf(stderr, "bad start position \n");
        }
        fclose(gridFile)
    } else {
        fprintf(stderr, "Could not open file \n");
    }
    return result;
}

void displayGrid(Grid grid, Position creaturePosition, int* genome, int genomeLength, int geneIndex) {
    
    for (int i = 0; i < genomeLength; i++) {
        swtich (genome[i]) {
#define MOVE(dX, dY, intValue, character) \
            case intValue:\
                printf(character);\
                break;
#include "move.txt"
            default:
                fprintf(stderr, "unexpected inexistant genome code\n");
        }
    }
    printf("\n");
    for (int i = 0; i < genomeLength; i++) {
        if (i == geneIndex) {
            printf("^");
        } else {
            printf(" ");
        }
    }
    printf("\n\n");
    
    // Do we use # or this?
	
    static const char face[4] = {0xE2, 0x98, 0xBB, '\0'};
    static const char start[4] = {'S', '\0'};
    static const char finish[4] = {'F', '\0'};
    static const char obstacleCenter[4] = {'X', '\0'};
    
    printf("+");
    for (int j = 0; j < grid.width; j++) {
        printf("---+");
    }
    printf("\n");
    for (Position p = {0, 0}; p.y < grid.height; p.y++) {
        printf("|");
        for (p.x = 0; p.x < grid.width; p.x++) {
            // ___|
            printf(" ");
            if (equalPos(p, creaturePosition)) {
                printf(face);
            } else if (equalPos(p, grid.start)) {
                printf(start);
            } else if (equalPos(p, grid.finish)) {
                printf(finish);
            } else if (getInGrid(grid, p) == obstacle) {
                printf(obstacleCenter); // distinction between obstacles and free place with obstacles around
            } else {
                printf(" ");
            }
            printf(" ");
            
            Position nextP = {p.x + 1, p.y};
            
            if (nextP.x == grid.width) {
                printf("|");
            } else if (getInGrid(grid, p) == obstacle || getInGrid(grid, nextP) == obstacle) {
                printf("|");
            } else {
                printf(" ");
            }
        }
        printf("\n");
        
        printf("+");
        for (p.x = 0; p.x < grid.width; p.x++) {
            Position underP = {p.x, p.y + 1}, nextP = {p.x + 1, p.y}, underNextP = {p.x + 1, p.y + 1};
            // ---+
            if (underP.y == grid.height) {
                printf("---");
            } else if (getInGrid(grid, p) == obstacle || getInGrid(grid, underP) == obstacle) {
                printf("---");
            } else {
                printf("   ");
            }
            
            if (underP.y == grid.height || nextP.x == grid.width) {
                printf("+");
            } else if (getInGrid(grid, p) == obstacle ||
                       getInGrid(grid, underP) == obstacle ||
                       getInGrid(grid, nextP) == obstacle ||
                       getInGrid(grid, underNextP) == obstacle) {
                printf("+");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}
