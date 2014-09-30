/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>

using namespace std;

#define TILE_DROP_SPEED 200
#define TILE_DROP_SPEED_FAST TILE_DROP_SPEED/10
#define MAX_TILE_SHAPES 3
#define MAX_TILE_ORIENTATIONS 4

enum TileInfo {
	TILE_CREATE,
	TILE_TICK
};

enum TileShape {
	TILE_SHAPE_I,
	TILE_SHAPE_S,
	TILE_SHAPE_L
};

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

int tileDropSpeed = TILE_DROP_SPEED;
// current tile
vec2 currTile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 currTilePos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
vec2 currTileShape[4][4];
TileShape currTileShapeType = TILE_SHAPE_I;
int currTileOrientation = 0;
vec4 currTileColours[4];

//-------------------------------------------------------------------------------------------------------------------

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
// all are in a,b,c,d order
vec2 allRotationsIshape[4][4] =
	{{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},
	{vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)},
	{vec2(-2, 0), vec2(-1, 0), vec2(0, 0), vec2(1, 0)},
	{vec2(0, -2), vec2(0, -1), vec2(0, 0), vec2(0, 1)}};
vec2 allRotationsSshape[4][4] =
	{{vec2(-1, -1), vec2(0, -1), vec2(0, 0), vec2(1, 0)},
	{vec2(1, -1), vec2(1, 0), vec2(0, 0), vec2(0, 1)},
	{vec2(-1, -1), vec2(0, -1), vec2(0, 0), vec2(1, 0)},
	{vec2(1, -1), vec2(1, 0), vec2(0, 0), vec2(0, 1)}};
vec2 allRotationsLshape[4][4] = 
	{{vec2(-1, -1), vec2(-1,0), vec2(0, 0), vec2(1, 0)},
	{vec2(1, -1), vec2(0, -1), vec2(0, 0), vec2(0, 1)},     
	{vec2(1, 1), vec2(1, 0), vec2(0, 0), vec2(-1,  0)},  
	{vec2(-1, 1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

void changeTileFromAtoB(vec2 from[][4], vec2 to[][4]) {
	for(int i = 0; i < 4; i++) {
		for(int k = 0; k < 4; k++) {
			from[i][k].x = to[i][k].x;
			from[i][k].y = to[i][k].y;
		}
	}
}

TileShape setTileShape(vec2 _tile[][4], TileShape type) {
	switch(type) {
		case TILE_SHAPE_I: changeTileFromAtoB(_tile, allRotationsIshape); break;
		case TILE_SHAPE_S: changeTileFromAtoB(_tile, allRotationsSshape); break;
		case TILE_SHAPE_L: changeTileFromAtoB(_tile, allRotationsLshape); break;
		default: changeTileFromAtoB(_tile, allRotationsIshape); break;
	}
	return type;
}

TileShape setRandTileShape(vec2 _tile[][4]) {
	return setTileShape(_tile, (TileShape)(rand() % MAX_TILE_SHAPES));
}

//-------------------------------------------------------------------------------------------------------------------

// colors
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); 

// fruit colors: https://kuler.adobe.com/create/color-wheel/?base=2&rule=Custom&selected=3&name=My%20Kuler%20Theme&mode=rgb&rgbvalues=1,0.8626810137791381,0,0.91,0.5056414909356977,0,1,0.10293904996979109,0,0.5587993310653088,0,0.91,0.1658698853207745,1,0.10159077034733333&swatchOrder=0,1,2,3,4
vec4 grape  = vec4(142/255.0 , 54/255.0  , 232/255.0 , 1.0);
vec4 apple  = vec4(255/255.0 , 26/255.0  , 0/255.0  , 1.0);
vec4 banana = vec4(255/255.0 , 220/255.0 , 0/255.0  , 1.0);
vec4 pear   = vec4(42/255.0  , 255/255.0 , 26/255.0  , 1.0);
vec4 orange = vec4(232/255.0 , 129/255.0 , 0/255.0   , 1.0);

vec4 randFruitColor() {
	//return vec4(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, 1.0);
	vec4 randc;
	switch(rand() % 5) {
		case 0: randc = grape; break;
		case 1: randc = apple; break;
		case 2: randc = banana; break;
		case 3: randc = pear; break;
		case 4: randc = orange; break;
	}
	return randc;
}

 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
enum VBO_IDs {
	GridPositionBO,
	GridColourBO,
	BoardPositionBO,
	BoardColourBO,
	CurrentTilePositionBO,
	CurrentTileColourBO
};
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = currTilePos.x + currTile[i].x; 
		GLfloat y = currTilePos.y + currTile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called to keep the tile within the bounds of the board by nudging the tile into place
void nudge(int cellOffsetX, int cellOffsetY) {
	int cellX = currTilePos.x + cellOffsetX;
	int cellY = currTilePos.y + cellOffsetY;
	currTilePos.x -= cellX<0 ? cellOffsetX : cellX>9 ? cellOffsetX : 0;
	currTilePos.y -= cellY>19 ? cellOffsetY : 0;
}

//-------------------------------------------------------------------------------------------------------------------

void shuffleColours() {
	vec4 temp = currTileColours[0];
	currTileColours[0] = currTileColours[1];
	currTileColours[1] = currTileColours[2];
	currTileColours[2] = currTileColours[3];
	currTileColours[3] = temp;
	
	// Update the color VBO of current tile
	vec4 newcolours[24];
	for (int i = 0; i < 24; i++)
		newcolours[i] = currTileColours[i/6];

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	tileDropSpeed = TILE_DROP_SPEED;
	//currTilePos = vec2(5 , 19); // Put the tile at the top of the board
	currTilePos = vec2(rand() % 10 + 1 , 19); // Put the tile at the top of the board

	// Update the geometry VBO of current tile
	currTileShapeType = setRandTileShape(currTileShape);
	currTileOrientation = rand() % MAX_TILE_ORIENTATIONS;
	for (int i = 0; i < 4; i++) {
		currTile[i] = currTileShape[currTileOrientation][i]; // Get the 4 pieces of the new tile
		nudge(currTile[i].x, currTile[i].y);
	}
	updatetile(); 
	for (int i = 0; i < 4; i++) {
		currTileColours[i] = randFruitColor();
	}
	shuffleColours();

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = white; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1); // bottom left
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1); // bottom right
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1); // bottom right
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1); // top right
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{      
	int nextORIENid = (currTileOrientation + 1) % MAX_TILE_ORIENTATIONS;
	vec2 nextOrientation[4]; 
	for (int i = 0; i < MAX_TILE_ORIENTATIONS; i++)
		nextOrientation[i] = currTileShape[nextORIENid][i];
	for(int i = 0; i < 4; i++) {
		int cellX = currTilePos.x + nextOrientation[i].x;
		int cellY = currTilePos.y + nextOrientation[i].y;
		bool xValid = cellX <= 9 && cellX >= 0;
		bool yValid = cellY <= 19 && cellY >= 0;
		if(!(xValid && yValid))
			return;
		if(board[cellX][cellY] == true)
			return;
	}
	currTileOrientation = nextORIENid;
	for (int i = 0; i < 4; i++)
		currTile[i] = nextOrientation[i];
}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{

}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{

	for(int i = 0; i < 4; i++) {
		int cellX = currTilePos.x + currTile[i].x;
		int cellY = currTilePos.y + currTile[i].y;
		board[cellX][cellY] = true;

		boardcolours[6*(10*cellY + cellX)    ] = currTileColours[i];
		boardcolours[6*(10*cellY + cellX) + 1] = currTileColours[i];
		boardcolours[6*(10*cellY + cellX) + 2] = currTileColours[i];
		boardcolours[6*(10*cellY + cellX) + 3] = currTileColours[i];
		boardcolours[6*(10*cellY + cellX) + 4] = currTileColours[i];
		boardcolours[6*(10*cellY + cellX) + 5] = currTileColours[i];
	}

	// Bind the VBO containing current board colors
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[BoardColourBO]); 
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction) {
	for (int i = 0; i < 4; i++) {
		int cellX = currTilePos.x + currTile[i].x + direction.x;
		int cellY = currTilePos.y + currTile[i].y + direction.y;
		if(cellX < 0 || cellX > 9)
			return false;
		if(cellY < 0 || cellY > 19)
			return false;
		if(board[cellX][cellY] == true)
			return false;
	}
	return true;
}
//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{

}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------
// Handle arrow key keypresses
void special(int key, int x, int y)
{
	switch(key) {
		case GLUT_KEY_UP:
			rotate();
			cout << "GLUT_KEY_UP" << endl;
			break;
		case GLUT_KEY_DOWN:
			tileDropSpeed = TILE_DROP_SPEED_FAST;
			cout << "GLUT_KEY_DOWN" << endl;
			break;
		case GLUT_KEY_RIGHT:
			if(movetile(vec2(1, 0))) {
				cout << "GLUT_KEY_RIGHT" << endl;
				currTilePos.x += 1;
			}
			break;
		case GLUT_KEY_LEFT:
			if(movetile(vec2(-1, 0))) {
				cout << "GLUT_KEY_LEFT" << endl;
				currTilePos.x -= 1;
			}
			break;
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case ' ':
			shuffleColours();
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

bool freeToFall() {
	for(int i = 0; i < 4; i++) {
		nudge(currTile[i].x, currTile[i].y);
		int cellX = currTilePos.x + currTile[i].x;
		int cellY = currTilePos.y + currTile[i].y;
		//cout << "cellX: " << cellX << " cellY: " << cellY << endl;
		//if(cellX > 9)
		//	currTilePos.x -= currTile[i].x;
		if(cellY - 1 < 0)
			return false;
		if(board[cellX][cellY - 1] == true)
			return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------

void tileDrop(int type) {
	TileInfo value = TILE_TICK;
	switch(type) {
		case TILE_CREATE: cout << "TILE_CREATE" << endl; break;
		case TILE_TICK: cout << "TILE_TICK" << endl; break;
		default: break;
	}

	if(freeToFall()) {
		currTilePos.y -= 1;
	} else {
		settile();
		newtile();
		value = TILE_CREATE;
	}
	updatetile();
	glutTimerFunc(tileDropSpeed, tileDrop, value);
}

//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	glutTimerFunc(tileDropSpeed, tileDrop, TILE_CREATE);

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}

