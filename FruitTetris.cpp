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

/* Steven Huang
 * 301223245
 * sha152
 */

#include "include/Angel.h"
#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "robot.h"

using namespace std;

// misc constants
#define TILE_DROP_SPEED 200
#define TILE_DROP_SPEED_FAST 20
#define MAX_TILE_ORIENTATIONS 4
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define MAX_FRUIT_GROUP 3
#define BOARD_POINTS 1200*6
#define MAX_GRIP_TIME 5

// forward declarations
int checkFullRow(const vec2 &p);
void recursiveCheck(const vec2 &p, const vec2 &dir, vector<vec2> *h, vector<vec2> *v);
void checkGroupedFruits(const vec2 &p);
void tileDrop(int type);
void rotateCurrentTile(int n);
bool isInBoardBounds(vec2 p);
bool isInBoardBounds(int x, int y);
void updateTileColours();
vec4 getCellColour(const vec2 &p);

// information to draw to screen
enum Text {
	TextScore,
	TextCells,
	TextRows,
	TextGG,
	GripTime,
	TextMax
};
float gui[TextMax];

enum TileInfo {
	TILE_CREATE,
	TILE_TICK,
	TILE_TICK_FAST,
	TILE_COLUMN_CHECK
};
bool tileFalling = false;
// various callback variables to make sure only 1 callback is called at once
int numCheckFruitColumnCallbacks = 0;
int numDropTileCallbacks = 0;
int numFastDropTileCallbacks = 0;
// vector of removed cells to perform alpha modifications on
vector<vec2> cellsToAnimate;
// vector of removed cells to perform column drops on
vector<vec2> removedCells;

// vector sorting
struct sortByDecY { bool operator() (vec2 const &L, vec2 const &R) { return L.y > R.y; } };
struct sortByIncY { bool operator() (vec2 const &L, vec2 const &R) { return L.y < R.y; } };

// Debug prints
void printVec4(const vec4 &f) {
	cout.precision(6);
	cout << fixed << "Vec4: " << f.x << " " << f.y << " " <<f.z <<" "<<f.w << endl;
}
void printVec2(const vec2 &f) { cout << "(" << f.x << "," << f.y << ")"; }
bool vec4Equal(const vec4 &a, const vec4 &b) {
	return a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w;
}

int tileDropSpeed = TILE_DROP_SPEED;
// current tile
vec2 currTileOffset[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 currTilePos = vec2(5, BOARD_HEIGHT - 1); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
int currTileShapeIndex = 0;
vec4 currTileColours[4];

//-------------------------------------------------------------------------------------------------------------------
// TileShape enum of list of valid shapes
enum TileShape {
	TileShapeI,
	TileShapeS,
	TileShapeZ,
	TileShapeO,
	TileShapeT,
	TileShapeJ,
	TileShapeL,
	MaxTileShapes
};
const vec2 allShapes[MaxTileShapes][4] =
	{{vec2(-2,  0), vec2(-1,  0), vec2(0, 0), vec2( 1,  0)},  // I
	 {vec2(-1, -1), vec2( 0, -1), vec2(0, 0), vec2( 1,  0)},  // S
	 {vec2( 1, -1), vec2( 0, -1), vec2(0, 0), vec2(-1,  0)},  // Z
	 {vec2(-1, -1), vec2( 0, -1), vec2(0, 0), vec2(-1,  0)},  // O
	 {vec2(-1,  0), vec2( 1,  0), vec2(0, 0), vec2( 0,  1)},  // T
	 {vec2(-1,  1), vec2(-1,  0), vec2(0, 0), vec2( 1,  0)},  // J
	 {vec2(-1, -1), vec2(-1,  0), vec2(0, 0), vec2( 1,  0)}}; // L
//-------------------------------------------------------------------------------------------------------------------
const vec4 white          = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 grey           = vec4(0.6, 0.6, 0.6, 1.0);
const vec4 gridColour     = vec4(0.8, 0.8, 0.8, 0.8);
const vec4 black          = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 cellFreeColour = vec4(1.0, 1.0, 1.0, 0.0);
// fruit colors: https://kuler.adobe.com/create/color-wheel/?base=2&rule=Custom&selected=3&name=My%20Kuler%20Theme&mode=rgb&rgbvalues=1,0.8626810137791381,0,0.91,0.5056414909356977,0,1,0.10293904996979109,0,0.5587993310653088,0,0.91,0.1658698853207745,1,0.10159077034733333&swatchOrder=0,1,2,3,4
const vec4 grape  = vec4(142/255.0 ,  54/255.0 , 232/255.0 , 1.0);
const vec4 apple  = vec4(255/255.0 ,  26/255.0 ,   0/255.0 , 1.0);
const vec4 banana = vec4(255/255.0 , 220/255.0 ,   0/255.0 , 1.0);
const vec4 pear   = vec4( 42/255.0 , 255/255.0 ,  26/255.0 , 1.0);
const vec4 orange = vec4(232/255.0 , 129/255.0 ,   0/255.0 , 1.0);
enum FruitColours {
	ColourGrape,
	ColourApple,
	ColourBanana,
	ColourPear,
	ColourOrange,
	MaxFruitColours
};
const vec4 fruitColours[MaxFruitColours] = {grape, apple, banana, pear, orange};
//-------------------------------------------------------------------------------------------------------------------
 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[BOARD_WIDTH][BOARD_HEIGHT]; 

void setCellOccupied(const vec2 &p, bool o) {
	board[(int)p.x][(int)p.y] = o;
}
void setCellOccupied(int x, int y, bool o) {
	board[x][y] = o;
}
bool isCellOccupied(int x, int y) {
	if(!isInBoardBounds(x, y))
		return false;
	return board[x][y];
}
bool isCellOccupied(const vec2 &p) {
	if(!isInBoardBounds(p))
		return false;
	return board[(int)p.x][(int)p.y];
}

//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[BOARD_POINTS];

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// alpha value for fade out animation upon game over
float fadeOut = 1.0f;

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locMVP;

void setMVP(mat4 &mvp) {
	glUniformMatrix4fv(locMVP, 1, GL_TRUE, mvp);
}

mat4 Projection,View, Model;

// VAO and VBO
enum VAO_IDs {
	VAOGrid,
	VAOBoard,
	VAOTile,
	MaxVaoIds
};
GLuint vaoIDs[MaxVaoIds]; // One VAO for each object: the grid, the board, the current piece;
enum VBO_IDs {
	GridPositionBO,
	GridColourBO,
	BoardPositionBO,
	BoardColourBO,
	CurrentTilePositionBO,
	CurrentTileColourBO,
	MaxVboIds
};
GLuint vboIDs[MaxVboIds]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
bool isAboveBoard(vec2 p) {
	if(p.x < 0 || p.x > BOARD_WIDTH - 1) return false;
	if(p.y < 0) return false;
	return true;
}
bool isInBoardBounds(vec2 p) {
	if(p.x < 0 || p.x > BOARD_WIDTH - 1) return false;
	if(p.y < 0 || p.y > BOARD_HEIGHT - 1) return false;
	return true;
}
bool isInBoardBounds(int x, int y) {
	return isInBoardBounds(vec2(x, y));
}
int canRelease() {
	int cellsInBoard = 0; int cellsOccupied = 0;
	for(int i = 0; i < 4; i++) {
		vec2 p = currTilePos + currTileOffset[i];
		if(isCellOccupied(p)) cellsOccupied++;
		if(isAboveBoard(p)) cellsInBoard++;
	}
	return cellsInBoard == 4 && cellsOccupied == 0;
}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile() {
	if(gui[TextGG]) return;
	if(!tileFalling)
		currTilePos = robot::getTip();

	if(canRelease()) {
		updateTileColours();
	} else {
		vec4 newcolours[24*6];
		for (int i = 0; i < 24*6; i++)
			newcolours[i] = grey;
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[CurrentTileColourBO]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	}

	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[CurrentTilePositionBO]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = currTilePos.x + currTileOffset[i].x; 
		GLfloat y = currTilePos.y + currTileOffset[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 16.50, 1); // front left bottom
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 16.50, 1); // front left top
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 16.50, 1); // front right bottom
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 16.50, 1); // front right top
		vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), -16.50, 1); // back left bottom
		vec4 p6 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), -16.50, 1); // back left top
		vec4 p7 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), -16.50, 1); // back right bottom
		vec4 p8 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), -16.50, 1); // back right top

		// Two points are used by two triangles each
		vec4 newpoints[36] = {	p1, p2, p3, p2, p3, p4,
								p5, p6, p7, p6, p7, p8,
								p1, p2, p5, p2, p5, p6,
								p3, p4, p7, p4, p7, p8,
								p2, p4, p6, p4, p6, p8,
								p1, p3, p5, p3, p5, p7};

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*sizeof(newpoints), sizeof(newpoints), newpoints); 
	}
}

//-------------------------------------------------------------------------------------------------------------------

// Called to keep the tile within the bounds of the board by nudging the tile into place
bool nudgeCurrentTile(const vec2 *o) {
	for(int i = 0; i < 4; i++) {
		vec2 p = currTilePos + o[i];
		if(isInBoardBounds(p) && isCellOccupied(p)) return false;
	}
	for(int i = 0; i < 4; i++) {
		vec2 p = currTilePos + o[i];
		if(!isInBoardBounds(p)) currTilePos -= o[i];
	}
	return true;
}
bool nudgeCurrentTile(int cellOffsetX, int cellOffsetY) {
	int cellX = currTilePos.x + cellOffsetX;
	int cellY = currTilePos.y + cellOffsetY;
	if(isInBoardBounds(cellX,cellY) && isCellOccupied(cellX, cellY)) return false;
	currTilePos.x -= cellX<0 ? cellOffsetX : cellX>BOARD_WIDTH - 1 ? cellOffsetX : 0;
	currTilePos.y -= cellY>BOARD_HEIGHT - 1 ? cellOffsetY : cellY<0 ? cellOffsetY : 0;
	return true;
}

//-------------------------------------------------------------------------------------------------------------------

void updateTileColours() {
	// Update the color VBO of current tile
	vec4 newcolours[24*6];
	for (int i = 0; i < 24*6; i++)
		newcolours[i] = currTileColours[i/6/6];
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[CurrentTileColourBO]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void shuffleAndUpdateColours() {
	vec4 temp = currTileColours[0];
	for(int i = 0; i < 4 - 1; i++)
		currTileColours[i] = currTileColours[i + 1];
	currTileColours[3] = temp;
	
	updateTileColours();
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile() {
	if(gui[TextGG]) return;
	tileDropSpeed = TILE_DROP_SPEED;
	//vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 16.50, 1); // front left bottom
	//currTilePos = vec2(rand() % BOARD_WIDTH, BOARD_HEIGHT - 1); // Put the tile at the top of the board
	currTilePos = robot::getTip();

	// Update the geometry VBO of current tile
	currTileShapeIndex = rand() % MaxTileShapes;
	for(int i = 0; i < 4; i++) {
		currTileColours[i] = fruitColours[rand() % MaxFruitColours];
		currTileOffset[i] = allShapes[currTileShapeIndex][i];
		//nudgeCurrentTile(currTileOffset[i].x, currTileOffset[i].y);
	}
	rotateCurrentTile(rand() % 5);
	shuffleAndUpdateColours();
	updatetile(); 

	// can't lose! ;)
	/*for(int i = 0; i < 4; i++) {
		if(isCellOccupied(currTilePos + currTileOffset[i])) {
			gui[TextGG] = 1;
		}
	}*/
	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid() {
	// 462 = 21*11*2 is depth lines
	vec4 gridpoints[64*2 + 462];
	vec4 gridcolours[64*2 + 462];
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i]      = vec4((33.0 + (33.0 * i)), 33.0, 16.50, 1);
		gridpoints[2*i + 1]  = vec4((33.0 + (33.0 * i)), 693.0, 16.50, 1);
		gridpoints[2*i + 64] = vec4((33.0 + (33.0 * i)), 33.0, -16.50, 1);
		gridpoints[2*i + 65] = vec4((33.0 + (33.0 * i)), 693.0, -16.50, 1);
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] 		= vec4(33.0, (33.0 + (33.0 * i)), 16.50, 1);
		gridpoints[22 + 2*i + 1] 	= vec4(363.0, (33.0 + (33.0 * i)), 16.50, 1);
		gridpoints[22 + 2*i + 64]	= vec4(33.0, (33.0 + (33.0 * i)), -16.50, 1);
		gridpoints[22 + 2*i + 65] 	= vec4(363.0, (33.0 + (33.0 * i)), -16.50, 1);
	}
	// Depth lines
	for (int i = 0; i < BOARD_HEIGHT + 1; i++){
		for (int j = 0; j < BOARD_WIDTH + 1; j++) {
			gridpoints[128 + 22*i + 2*j] 		= vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), 16.50, 1); // front left bottom
			gridpoints[128 + 22*i + 2*j + 1] 	= vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1); // back left bottom
		}
	}
	// Make all grid lines coloured
	for (int i = 0; i < 64*2 + 462; i++)
		gridcolours[i] = gridColour;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[VAOGrid]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[GridPositionBO]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, (128 + 462)*sizeof(vec4), gridpoints, GL_DYNAMIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[GridColourBO]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, (128 + 462)*sizeof(vec4), gridcolours, GL_DYNAMIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}

void face(vec4 *boardpoints, int index, vec4 &p1, vec4 &p2, vec4 &p3, vec4 &p4) {
	boardpoints[index 	 ] = p1;
	boardpoints[index + 1] = p2;
	boardpoints[index + 2] = p3;
	boardpoints[index + 3] = p2;
	boardpoints[index + 4] = p3;
	boardpoints[index + 5] = p4;
}

void initBoard() {
	// *** Generate the geometric data
	vec4 boardpoints[BOARD_POINTS];
	for (int i = 0; i < BOARD_POINTS; i++)
		boardcolours[i] = cellFreeColour;
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < BOARD_HEIGHT; i++){
		for (int j = 0; j < BOARD_WIDTH; j++)
		{
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), 16.50, 1); // front left bottom
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), 16.50, 1); // front left top
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), 16.50, 1); // front right bottom
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), 16.50, 1); // front right top
			vec4 p5 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1); // back left bottom
			vec4 p6 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), -16.50, 1); // back left top
			vec4 p7 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1); // back right bottom
			vec4 p8 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), -16.50, 1); // back right top
			
			int index = 36*(BOARD_WIDTH*i + j);
			face(boardpoints, index     , p1, p2, p3, p4); // front
			face(boardpoints, index + 6 , p5, p6, p7, p8); // back
			face(boardpoints, index + 12, p1, p2, p5, p6); // left
			face(boardpoints, index + 18, p3, p4, p7, p8); // right
			face(boardpoints, index + 24, p2, p4, p6, p8); // up
			face(boardpoints, index + 30, p1, p3, p5, p7); // down
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < BOARD_WIDTH; i++)
		for (int j = 0; j < BOARD_HEIGHT; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[VAOBoard]);
	glGenBuffers(2, &vboIDs[BoardPositionBO]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[BoardPositionBO]);
	glBufferData(GL_ARRAY_BUFFER, BOARD_POINTS*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[BoardColourBO]);
	glBufferData(GL_ARRAY_BUFFER, BOARD_POINTS*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile() {
	glBindVertexArray(vaoIDs[VAOTile]);
	glGenBuffers(2, &vboIDs[CurrentTilePositionBO]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[CurrentTilePositionBO]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[CurrentTileColourBO]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init() {
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
	robot::init();
	robot::Theta[robot::LowerArm] = 5;
	robot::Theta[robot::UpperArm] = -85;

	// The location of the uniform variables in the shader program
	locMVP = glGetUniformLocation(program, "MVP");

	// Board is now in unit lengths
	vec3 topOfBoard = vec3(0, BOARD_HEIGHT + 10, 24);
	vec3 centerOfBoard = vec3(0, BOARD_HEIGHT/2, 0);
	View = LookAt(
			topOfBoard,
			centerOfBoard,
			vec3(0, 1, 0));

	// Game initialization
	// reset variables 
	fadeOut = 1.0f;
	gui[TextGG] = 0;
	gui[TextScore] = 0;
	gui[TextCells] = 0;
	gui[TextRows] = 0;
	gui[GripTime] = MAX_GRIP_TIME;

	newtile(); // create new next tile

	// Blend
   	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1, 1, 1, 1);
	// Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);
	// Antialiasing
	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, nudge to make room
void rotateCurrentTile(int n) {
	if(currTileShapeIndex == TileShapeO) { shuffleAndUpdateColours(); return; }
	vec2 nextOrientation[4];
	for(int i = 0; i < 4; i++) nextOrientation[i] = vec2(currTileOffset[i].y, -currTileOffset[i].x);
	// rotate additional n times for random rotation
	for(int k = 0; k < n; k++) 
		for(int i = 0; i < 4; i++) nextOrientation[i] = vec2(nextOrientation[i].y, -nextOrientation[i].x);
	// if cannot nudge tile back into valid bounds, cancel rotation
	if(!nudgeCurrentTile(nextOrientation)) return;
	// otherwise apply this rotation
	for(int i = 0; i < 4; i++) currTileOffset[i] = nextOrientation[i];
}

//-------------------------------------------------------------------------------------------------------------------
// sets colour of the specified board to c
void setCellColour(const vec2 &p, const vec4 &c) {
	for(int i = 0; i < 36; i++) {
		int index = 36*(BOARD_WIDTH*(int)p.y + (int)p.x) + i;
		boardcolours[index] = c;
	}
}
void setCellColour(int x, int y, const vec4 &c) {
	setCellColour(vec2(x, y), c);
}

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void setTileColour(const vec2 &p) {
	for(int i = 0; i < 4; i++) {
		int cellX = p.x + currTileOffset[i].x;
		int cellY = p.y + currTileOffset[i].y;
		board[cellX][cellY] = true;
		setCellColour(cellX, cellY, currTileColours[i]);
	}
}

vec4 getCellColour(const vec2 &p) {
	return boardcolours[36*(10*(int)p.y + (int)p.x)];
}

// checks if cell is able to fall, returns true if able, false otherwise
bool cellFreeToFall(const vec2 &p) {
	if((int)p.y - 1 < 0) return false;
	return !isCellOccupied(p - vec2(0, 1));
}

bool tileFreeToFall(const vec2 &p) {
	for(int i = 0; i < 4; i++) {
		if(!cellFreeToFall(p + currTileOffset[i]))
			return false;
	}
	return true;
}

void updateBoard() {
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[BoardColourBO]); 
	glBufferData(GL_ARRAY_BUFFER, BOARD_POINTS*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool moveTile(vec2 direction) {
	for (int i = 0; i < 4; i++) {
		vec2 newPos = currTilePos + currTileOffset[i] + direction;
		if(!isInBoardBounds(newPos) || isCellOccupied(newPos))
			return false;
	}
	return true;
}
//-------------------------------------------------------------------------------------------------------------------

// generically draws text to screen
template<class T>
void drawText(T str, float x, float y) {
	stringstream ss(str);
	glRasterPos2f(x, y);
	char c;
	while(ss >> noskipws >> c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	numDropTileCallbacks = numFastDropTileCallbacks = numCheckFruitColumnCallbacks = 0;
	init();
}
//-------------------------------------------------------------------------------------------------------------------

// moving text!
float x = -1.0f;
float y = 0.7f;
// Draws the game
void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor4f(1.0f, 0.0f, 0.0f, fadeOut);

	Projection = Perspective(45, 1.0*xsize/ysize, 10, 200);

	// Draw the robot
    glBindVertexArray(robot::vao);
	mat4 f = Projection * View * Translate(robot::pos);
	robot::robotMVP = RotateY(robot::Theta[robot::Base] );
	robot::base(f);

	robot::robotMVP *= Translate(0.0, robot::BASE_HEIGHT, 0.0);
	robot::robotMVP *= RotateZ(robot::Theta[robot::LowerArm]);
	robot::lower_arm(f);

	robot::robotMVP *= Translate(0.0, robot::LOWER_ARM_HEIGHT, 0.0);
	robot::robotMVP *= RotateZ(robot::Theta[robot::UpperArm]);
	robot::upper_arm(f);

	robot::robotMVP *= Translate(0.0, robot::UPPER_ARM_HEIGHT, 0.0);

	// Scale everything to unit length
	mat4 Model = mat4();
	Model *= Translate(0, BOARD_HEIGHT/2.0, 0);
	Model *= Scale(1.0/33, 1.0/33, 1.0/33);  // scale to unit length
	Model *= Translate(-33*BOARD_WIDTH/2.0 - 33, -33*BOARD_HEIGHT/2.0 - 33, 0); // move to origin

	mat4 MVP = Projection * View * Model;
	setMVP(MVP);

	glBindVertexArray(vaoIDs[VAOBoard]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, BOARD_POINTS); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[VAOTile]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24*6); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[VAOGrid]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 128 + 462);

	/*Draw deletion animation*/
	for(vector<vec2>::iterator cell = cellsToAnimate.begin(); cell != cellsToAnimate.end();) {
		vec4 lerp = getCellColour(*cell);
		if(lerp.w > 0.01 && !isCellOccupied(*cell)) {
			setCellColour(*cell, vec4(lerp.x, lerp.y, lerp.z, lerp.w - lerp.w*0.08));
			cell++;
		} else
			cell = cellsToAnimate.erase(cell);
	}
	updateBoard();
	
	// fade out everyhing while fading in the game over text
	if(gui[TextGG]) {
		// fade in GG text
		glColor4f(1.0f, 0.0f, 0.0f, 1 - fadeOut);
		drawText("Game over!", -0.1, 0);
		drawText("Press R to play again", -0.2, -0.2);
		// fade board
		for(int i = 0; i < BOARD_POINTS; i++)
			boardcolours[i] = boardcolours[i] - vec4(0,0,0,fadeOut*0.04); // Let the empty cells on the board be black
		// fade grid
		vec4 gridcolours[64]; // One colour per vertex
		for(int i = 0; i < 64; i++)
			gridcolours[i] = vec4(gridColour.x,gridColour.y,gridColour.z,fadeOut*0.04);
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[GridColourBO]); // Bind the second grid VBO (vertex colours)
		glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_DYNAMIC_DRAW); // Put the grid colours in the VBO
		// fade out tile
		for(int i = 0; i < 4; i++)
			currTileColours[i] -= vec4(0,0,0, fadeOut*0.04);
		updateTileColours();
		// decrement fadeOut
		fadeOut -= fadeOut > 0.01 ?  fadeOut * 0.05 : 0;
	}


	glColor4f(0.0f, 0.0f, 0.0f, fadeOut);
	stringstream ss;
	// noskipws to draw whitespace 
	ss << noskipws << "Tile position: " << currTilePos.x << ',' << currTilePos.y;
	drawText(ss.str(), -1, 0.95);

	drawText("have fun!", x+=0.001, y);

	ss.clear(); ss.str("");
	ss << noskipws << "Score: " << gui[TextScore] << " Cells Deleted: " << gui[TextCells] << " Rows Deleted: " << gui[TextRows];
	drawText(ss.str(), -0.5, -0.95);

	ss.clear(); ss.str("");
	if(gui[GripTime] > 0) {
		gui[GripTime] -= 1/60.0;
	} else {
		// if can't release, move arm to middle and release
		if(!canRelease()) {
			robot::Theta[robot::LowerArm] = 5;
			robot::Theta[robot::UpperArm] = -85;
			updatetile();
		}
		gui[GripTime] = MAX_GRIP_TIME;
		numFastDropTileCallbacks++;
		tileDrop(TILE_TICK_FAST);
	}
	ss << noskipws << "Gripper Time Remaining: " << gui[GripTime];
	drawText(ss.str(), -0.1, 0.95);

	glutSwapBuffers();
}

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h) {
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

// Handle arrow key keypresses
void special(int key, int x, int y) {
	switch(key) {
		case GLUT_KEY_UP:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				View *= RotateZ(10);
			else {
				rotateCurrentTile(0);
				updatetile();
			}
			break;
		case GLUT_KEY_DOWN:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				View *= RotateZ(-10);
			else if(numFastDropTileCallbacks == 0) {
				if(canRelease()) {
					numFastDropTileCallbacks++;
					tileDrop(TILE_TICK_FAST);
				}
			}
			break;
		case GLUT_KEY_RIGHT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				View *= RotateY(10);
			break;
		case GLUT_KEY_LEFT:
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
				View *= RotateY(-10);
			break;
		default:
			break;
	}
}

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y) {
	// various test cases. press t or z to find out!
	int test[] = {
		0, 0, ColourApple, 0, 1, ColourApple, 0, 2, ColourGrape, 0, 3, ColourGrape, 0, 4, ColourApple, 0, 5, ColourApple,
		3, 0, ColourApple, 3, 1, ColourApple,
		4, 0, ColourApple, 4, 1, ColourApple, 4, 2, ColourGrape,
		5, 0, ColourGrape, 5, 1, ColourGrape, 5, 2, ColourApple,
		7, 0, ColourGrape, 7, 1, ColourGrape, 7, 2, ColourApple
	};
	int test2[] = {
		4, 0, ColourApple, 4, 1, ColourGrape, 4, 2, ColourGrape,
		5, 0, ColourApple, 5, 1, ColourApple, 5, 2, ColourGrape,
		6, 1, ColourApple
	};
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
			if(glutGetModifiers() == GLUT_ACTIVE_CTRL) {
				shuffleAndUpdateColours();
				updatetile();
			} else {
				if(canRelease()) {
					numDropTileCallbacks++;
					glutTimerFunc(0, tileDrop, TILE_CREATE);
				}
			}
			break;
		case 'a':
			cout << "theta[lowerArm] = " << robot::Theta[robot::LowerArm] << endl;
			robot::Theta[robot::LowerArm] += 5;
			updatetile();
			break;
		case 'd':
			cout << "theta[lowerArm] = " << robot::Theta[robot::LowerArm] << endl;
			robot::Theta[robot::LowerArm] -= 5;
			updatetile();
			break;
		case 'w':
			cout << "theta[upperArm] = " << robot::Theta[robot::UpperArm] << endl;
			robot::Theta[robot::UpperArm] += 5;
			updatetile();
			break;
		case 's':
			cout << "theta[upperArm] = " << robot::Theta[robot::UpperArm] << endl;
			robot::Theta[robot::UpperArm] -= 5;
			updatetile();
			break;
		case 't':
			for(int i = 0; i < (int)sizeof(test)/(int)sizeof(int); i+=3) {
				cout << '(' << test[i] << ','<< test[i+1] <<") :"<<i<< endl;
				setCellColour(test[i], test[i+1], fruitColours[test[i+2]]);
				setCellOccupied(test[i], test[i+1], true);
			}
			break;
		case 'z':
			for(int i = 0; i < (int)sizeof(test2)/(int)sizeof(int); i+=3) {
				cout << '(' << test2[i] << ','<< test2[i+1] <<") :"<<i<< endl;
				setCellColour(test2[i], test2[i+1], fruitColours[test2[i+2]]);
				setCellOccupied(test2[i], test2[i+1], true);
			}
			break;
	}
	glutPostRedisplay();
}

void idle(void) {
	glutPostRedisplay();
}

// removes the cell at p from the board
void removeCellFromBoard(const vec2 &p) {
	gui[TextScore]+=5;
	gui[TextCells]++;
	setCellOccupied(p, false);
	cellsToAnimate.push_back(p);
}

// fills the appropriate vectors into h and v with the grouped cells
void recursiveCheck(const vec2 &p, const vec2 &dir, vector<vec2> *h, vector<vec2> *v) {
	if(dir.x == 0 && dir.y == 0 && isInBoardBounds(p)) {
		vector<vec2> vertGroup; vertGroup.push_back(vec2(p));
		vector<vec2> horzGroup; horzGroup.push_back(vec2(p));
		recursiveCheck(p, vec2(0, -1), NULL, &vertGroup); recursiveCheck(p, vec2(0, 1), NULL, &vertGroup);
		recursiveCheck(p, vec2(1, 0), &horzGroup, NULL); recursiveCheck(p, vec2(-1, 0), &horzGroup, NULL);
		// swaps the discoevered cells into h and v
		h->swap(horzGroup); v->swap(vertGroup);
	} else {
		if(isInBoardBounds(p + dir) && isCellOccupied(p + dir) && vec4Equal(getCellColour(p), getCellColour(p + dir))) {
			if(h) h->push_back(vec2(p + dir)); else v->push_back(vec2(p + dir));
			recursiveCheck(p + dir, dir, h, v);
		}
	}
}

//-------------------------------------------------------------------------------------------------------------------

// checks for removed cells in removedCells vector and moves the column down if needed, then recursively calls checkGroupedFruits
// for the shifted cells if needed
void checkFruitColumn() {
	// filled in previous iterations
	static vector<vec2> checkNext;
	for(vector<vec2>::iterator toCheck = checkNext.begin(); toCheck != checkNext.end();)  {
		// detect the marker. if the marker is detected, break and check the rest in next time this function is called (next tick)
		if(toCheck->x == -1 && toCheck->y == -1) {
			checkNext.erase(toCheck);
			break; 
		}
		gui[TextScore] += 10;
		printVec2(*toCheck);
		checkGroupedFruits(*toCheck);
		toCheck = checkNext.erase(toCheck);
	} // cout << endl;

	// sort by decreasing Ys to prioritize removal of lower cells first
	sort(removedCells.begin(), removedCells.end(), sortByDecY());
	// set used to make sure only one removal is done per tick per each column
	set<int> columnChecked;
	for(vector<vec2>::iterator hole = removedCells.begin(); hole != removedCells.end();) {
		if(columnChecked.insert(hole->x).second) { // successful insertion means this col hasn't been shifted down yet
			vec2 baseCellToCheck = vec2(hole->x, hole->y - 1);
			bool checkInNextIter = !isInBoardBounds(baseCellToCheck) || isCellOccupied(baseCellToCheck);
			// check to make sure we can still go down
			for(vector<vec2>::iterator h = removedCells.begin(); h != removedCells.end(); h++) {
				if(h->x == baseCellToCheck.x) {
					if(h->y == baseCellToCheck.y) {
						checkInNextIter = true;
						//cout << " FOUND IT ! " << endl;
					}
				} else continue;
			}
			for(int y = hole->y; y < BOARD_HEIGHT - 1; y++) {
				vec2 cellToBeDropped = vec2(hole->x, y + 1);
				vec2 cellToBeFilled = vec2(hole->x, y);
				if(isCellOccupied(cellToBeDropped) && !isCellOccupied(cellToBeFilled)) {
					setCellOccupied(cellToBeDropped, false);
					setCellOccupied(cellToBeFilled, true);
					setCellColour(cellToBeFilled, getCellColour(cellToBeDropped));
					setCellColour(cellToBeDropped, cellFreeColour);
					// make note to check for grouped fruits in next iteration
					if(checkInNextIter) checkNext.push_back(cellToBeFilled);
				}
			}
			hole = removedCells.erase(hole);
		} else hole++;
	}
	// push a marker for current list of cells
	checkNext.push_back(vec2(-1,-1));
	glutTimerFunc(tileDropSpeed, tileDrop, TILE_COLUMN_CHECK);
}

// checks using recursion for all cells in same column/row that are the same colour of the cell in position p
void checkGroupedFruits(const vec2 &p) {
	vector<vec2> group, horzGroup, vertGroup;;
	recursiveCheck(p, vec2(0, 0), &horzGroup, &vertGroup);
	cout << p.x << "," << p.y << "         horzGroup.size(): " << horzGroup.size();
	for(int k = 0; k < (int)horzGroup.size(); k++) cout << "  " << horzGroup[k].x << "," << horzGroup[k].y;
	cout << "         vertGroup.size(): " << vertGroup.size();
	for(int k = 0; k < (int)vertGroup.size(); k++) cout << "  " << vertGroup[k].x << "," << vertGroup[k].y; cout << endl;

	horzGroup.size() >= MAX_FRUIT_GROUP ? group.swap(horzGroup) : group.swap(vertGroup);

	for(int k = 0; group.size() >= MAX_FRUIT_GROUP && k < MAX_FRUIT_GROUP; k++) 
		if(!isCellOccupied(group[k])) { cout << "xxxxx found cell not uccupied" << endl; return; }
	for(int k = 0; group.size() >= MAX_FRUIT_GROUP && k < MAX_FRUIT_GROUP; k++) {
		removedCells.push_back(group[k]);
		removeCellFromBoard(group[k]);
	}
	numCheckFruitColumnCallbacks++;
	glutTimerFunc(tileDropSpeed, tileDrop, TILE_COLUMN_CHECK);
}

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
int checkFullRow(const vec2 &p) {
	int rowsRemoved = 0;
	int columnsHit = 0;
	for(int i = 0; i < BOARD_WIDTH && isCellOccupied(i, p.y); i++, columnsHit++);
	if(columnsHit == BOARD_WIDTH) {
		gui[TextScore] += 50;
		gui[TextRows]++;
		rowsRemoved++;
		for(int x = 0; x < BOARD_WIDTH; x++) {
			removeCellFromBoard(vec2(x, p.y));
			for(int y = p.y; y < BOARD_HEIGHT - 1; y++) {
				vec2 cellToBeDropped = vec2(x, y + 1);
				vec2 cellToBeFilled = vec2(x, y);
				if(isCellOccupied(cellToBeDropped)) {
					setCellOccupied(cellToBeDropped, false);
					setCellOccupied(cellToBeFilled, true);
					setCellColour(cellToBeFilled, getCellColour(cellToBeDropped));
					setCellColour(cellToBeDropped, cellFreeColour);
				}
			}
		}
		updateBoard();
	} 
	return rowsRemoved;
}

//-------------------------------------------------------------------------------------------------------------------

// main loop that handles the moving down of the tile and other game logic
void tileDrop(int type) {
	TileInfo value = TILE_TICK;
	/*cout << "Y: " << type << endl;
	cout << "numDropTileCallbacks         : " << numDropTileCallbacks << endl;
	cout << "numFastDropTileCallbacks     : " << numFastDropTileCallbacks << endl;
	cout << "numCheckFruitColumnCallbacks : " << numCheckFruitColumnCallbacks << endl;*/
	switch(type) {
		case TILE_CREATE: // fall-through
		case TILE_TICK:
			if(numDropTileCallbacks > 1) { numDropTileCallbacks--; return; }
			if(numDropTileCallbacks == 1) {
				if(tileFreeToFall(currTilePos)) {
					currTilePos.y -= 1;
					tileFalling = true;
					updatetile();
					glutTimerFunc(tileDropSpeed, tileDrop, value);
				} else {
					numFastDropTileCallbacks = 0;
					numDropTileCallbacks--;
					setTileColour(currTilePos);
					// disable tile deletion ;)
					/*
					vector<vec2> lowestYCellsFirst; 
					for(int i = 0; i < 4; i++) lowestYCellsFirst.push_back(currTilePos + currTileOffset[i]);
					sort(lowestYCellsFirst.begin(), lowestYCellsFirst.end(), sortByIncY());
					int rowOffset = 0;
					for(int i = 0; i < 4; i++) {
						rowOffset+=checkFullRow(lowestYCellsFirst[i] - vec2(0, rowOffset));
						vector<vec2> horzGroup, vertGroup;
						recursiveCheck(lowestYCellsFirst[i], vec2(0, 0), &horzGroup, &vertGroup);
						if(horzGroup.size() >= MAX_FRUIT_GROUP) {
							checkGroupedFruits(lowestYCellsFirst[i]);
						} else if(i == 3) {
							for(int k = 0; k < 4; k++) checkGroupedFruits(lowestYCellsFirst[k]);
						}
					} */
					newtile();
					tileFalling = false;
				}
				updateBoard();
			}	
			return;
		case TILE_TICK_FAST:
			if(numFastDropTileCallbacks > 1) { numFastDropTileCallbacks--; return; }
			if(numFastDropTileCallbacks == 1) {
				if(tileFreeToFall(currTilePos)){
					currTilePos.y -= 1;
					tileFalling = true;
					updatetile();
					glutTimerFunc(TILE_DROP_SPEED_FAST, tileDrop, TILE_TICK_FAST);
				} else {
					numDropTileCallbacks++;
					glutTimerFunc(TILE_DROP_SPEED, tileDrop, TILE_TICK);
				}
			}
			return;
		case TILE_COLUMN_CHECK:
			if(numCheckFruitColumnCallbacks > 1) { numCheckFruitColumnCallbacks--; return; }
			if(numCheckFruitColumnCallbacks == 1)
				checkFruitColumn();
			return;
		default: cout << "WARNING: erroneous call to tileDrop" << endl; return;;
	}
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
