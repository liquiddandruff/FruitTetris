#version 130

in  vec4 color;
out vec4  fColor;

void main() 
{ 
	// discard if alpha is 0.0
	if(color.w == 0.0)
		discard;
	fColor = color;
} 

