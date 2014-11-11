#include <iostream>
#include "robot.h"

using namespace std;

namespace robot {
GLuint vao;
GLuint buffer;
const int NumVertices = 36;
point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 0.4, 0.5, 0.2, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 12.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 11.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

vec3 pos;
// Shader transformation matrices
mat4  robotMVP;

// Array of rotation angles (in degrees) for each rotation axis
//enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;
GLfloat  Theta[NumAngles] = { 0.0 };

// Menu option values
const int  Quit = 4;

int Index = 0;
void init( void ) {
	Index = 0;
	pos = vec3(-10, 0, 0);
    colorcube();
    
    // Create a vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)) );
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void quad( int a, int b, int c, int d ) {
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void colorcube() {
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

vec2 getTip() {
	vec2 tip;
	// base
	tip.x += pos.x/2;
	tip.y += pos.y + BASE_HEIGHT;
	// lower arm
	tip.x += LOWER_ARM_HEIGHT * -sin(3.14159/180* Theta[LowerArm]);
	tip.y += LOWER_ARM_HEIGHT * cos(-3.14159/180* Theta[LowerArm]);
	// upper arm
	tip.x += (UPPER_ARM_HEIGHT-0.5) * -cos(3.14159/180* (90 - Theta[LowerArm] - Theta[UpperArm]));
	tip.y += (UPPER_ARM_HEIGHT-0.5) * sin(3.14159/180* (90 - Theta[LowerArm] - Theta[UpperArm]));
	// round
	tip.x = (int)(0.5 + tip.x);
	tip.y = (int)(0.5 + tip.y);
	
	return tip;
}

void base(const mat4 &vp) {
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *
		 Scale( BASE_WIDTH,
			BASE_HEIGHT,
			BASE_WIDTH ) );

    glUniformMatrix4fv( locMVP, 1, GL_TRUE, vp * robotMVP * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void lower_arm(const mat4 &vp) {
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) *
		      Scale( LOWER_ARM_WIDTH,
			     LOWER_ARM_HEIGHT,
			     LOWER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( locMVP, 1, GL_TRUE, vp * robotMVP * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void upper_arm(const mat4 &vp) {
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
		      Scale( UPPER_ARM_WIDTH,
			     UPPER_ARM_HEIGHT,
			     UPPER_ARM_WIDTH ) );

    glUniformMatrix4fv( locMVP, 1, GL_TRUE, vp * robotMVP * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}


} // namespace robot
