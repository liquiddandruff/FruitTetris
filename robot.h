#include "include/Angel.h"

extern GLuint locMVP;
extern GLuint vPosition, vColor;

namespace robot {

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

extern vec3 pos;
extern mat4 robotMVP;
extern GLuint vao;
extern const GLfloat BASE_HEIGHT;
extern const GLfloat BASE_WIDTH;
extern const GLfloat LOWER_ARM_HEIGHT;
extern const GLfloat LOWER_ARM_WIDTH;
extern const GLfloat UPPER_ARM_HEIGHT;
extern const GLfloat UPPER_ARM_WIDTH;
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
extern GLfloat Theta[NumAngles];

vec2 getTip();
void init();
void quad( int a, int b, int c, int d );
void colorcube();
void base(const mat4&);
void upper_arm(const mat4&);
void lower_arm(const mat4&);

} // namespace robot
