/*	A variation on a modern hello world. For OpenGL 4.X

	This is not the fastest way of doing this, though it is quite fast relatively speaking.

	Perry Kivolowitz
	Assistant Professor of Computer Science
	Carthage College
*/

#include <iostream>
#include <assert.h>
#include <vector>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>

#include "shader.h"

using namespace std;
using namespace glm;

#define	BAD_GL_VALUE	(GLuint(-1))

/*	These ought to be combined into a single umbrella rather than broken out.
*/

// For single triangle mode, the first value in each vector is in-built.
vector<vec3> positions;			// Where each triangle will be placed.
vector<float> offsets;			// An angle for each triangle.
vector<float> scaling_factors;	// A scaling factor for each triangle.

/*	Provides a random float linearly interpolated between min and max inclusively.
*/
inline float RandomNumber(float min, float max)
{
	return float(rand()) / float(RAND_MAX) * (max - min) + min;
}

/*	Derives a random position in space at which triangles will be placed.
*/

inline vec3 RandomPosition()
{
	float min = -8.0;
	float max = 8.0;

	return vec3(RandomNumber(min, max), RandomNumber(min, max), RandomNumber(-4.0f, 4.0f));
}

inline float D2R(float d)
{
	return d / 180.0f * pi<float>();
}

void MakeRandomPositions(int n)
{
	// Make sure the first one is at the origin.
	positions.push_back(vec3(0.0f, 0.0f, 0.0f));

	for (int counter = 0; counter < n; counter++)
		positions.push_back(RandomPosition());
}

void MakeRandomOffsets(int n)
{
	// Make sure the first one is at 0.
	offsets.push_back(0.0f);

	for (int counter = 0; counter < n; counter++)
		offsets.push_back(RandomNumber(0.0f, 360.0f));
}

void MakeRandomScalingFactors(int n)
{
	// Make sure the first one is at 1
	scaling_factors.push_back(1.0f);

	for (int counter = 0; counter < n; counter++)
		scaling_factors.push_back(RandomNumber(0.25f, 1.0f));
}

// Instantiate a Shader. See shader.cpp for support functions for using shaders.
Shader simple_shader;

// These should ALL be collected in some OO friendly constructs.

GLuint window_handle = BAD_GL_VALUE;
ivec2 window_size;
float window_aspect;
bool wireframe = false;
bool many_mode = false;

vector<vec2> sh_vertices;
vector<vec3> sh_colors;

GLuint vertex_coordinate_handle;
GLuint color_value_handle;
GLuint vertex_array_handle;

bool GLReturnedError(char * s)
{
	bool return_error = false;
	#ifdef _DEBUG
	GLenum glerror;
	while ((glerror = glGetError()) != GL_NO_ERROR)
	{
		return_error = true;
		cerr << s << ": " << gluErrorString(glerror) << endl;
	}
	#endif
	return return_error;
}

void ReshapeFunc(int w, int h)
{
	// Question for reader: Why is this 'if' statement here?
	if (h > 0)
	{
		window_size = ivec2(w, h);
		window_aspect = float(w) / float(h);
	}
}

void CloseFunc()
{
	window_handle = -1;
}

void KeyboardFunc(unsigned char c, int x, int y)
{
	float current_time = float(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f;

	switch (c)
	{
	case 'm':
		many_mode = !many_mode;
		break;

	case 'w':
		wireframe = !wireframe;
		break;

	case 'x':
	case 27:
		glutLeaveMainLoop();
		return;
	}
}

void TimerFunc(int param)
{
	if (window_handle != BAD_GL_VALUE)
	{
		glutTimerFunc(param, TimerFunc, param);
		glutPostRedisplay();
	}
}

void DisplayFunc()
{
	GLReturnedError("DisplayFunc - on entrance");

	glViewport(0, 0, window_size.x, window_size.y);
	glClearColor(0, 0, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

	float current_time = float(glutGet(GLUT_ELAPSED_TIME)) / 1000.0f;

	glm::mat4 projection_matrix = glm::perspective(D2R(many_mode ? 60.0f : 30.0f), float(window_size.x) / float(window_size.y), 1.0f, 30.0f);
	glm::mat4 modelview_matrix;
	
	if (many_mode)
		modelview_matrix = rotate(modelview_matrix, D2R(current_time * 10.0f) , vec3(0.0f , 1.0f , 0.0f));
	modelview_matrix = glm::lookAt(glm::vec3(2.0f * sin(current_time) , 0.0f , 10.0f) , glm::vec3(0.0f) , glm::vec3(0.0f , 1.0f , 0.0f)) * modelview_matrix;

	mat4 mvp = projection_matrix * modelview_matrix;


	simple_shader.Use();
	glBindVertexArray(vertex_array_handle);
	for (size_t counter = 0; counter < (many_mode ? positions.size() : 1); counter++)
	{
		mat4 m;
//		m = translate(m , vec3(2.0f * sin(current_time) , 0.0f , 0.0f));
//			translate(scale(mat4(), vec3(0.5f, 0.5f, 1.0f)), positions[counter]);
//		m = scale(m , vec3(scaling_factors[counter]));
//		m = rotate(m, D2R((60.0f + offsets[counter] / 10.0f) + offsets[counter]), vec3(0.0f, scaling_factors[counter], 1.0f));
		mat4 mvpm = mvp * m;
		glUniformMatrix4fv(simple_shader.mvp_handle, 1, GL_FALSE, glm::value_ptr(mvpm));
		glDrawArrays(GL_TRIANGLES, 0, (GLsizei)sh_vertices.size());
	}

	glBindVertexArray(0);
	glUseProgram(0);
	glFlush();
	GLReturnedError("DisplayFunc - on exit");
}

inline float D2R(float d)
{
	return d * glm::pi<float>() / 180.0f;
}

bool InitializeGeometry()
{
	GLReturnedError("InitializeGeometry - on entrance");
	int number_of_slices = 60;

	for (int i = 0; i < number_of_slices; i++)
	{

	}

	sh_vertices.push_back(glm::vec2(0.0f, 1.0f));
	sh_vertices.push_back(glm::vec2(-1.0f, -1.0f));
	sh_vertices.push_back(glm::vec2(1.0f, -1.0f));
	sh_vertices.push_back(vec2(1.0 , 1.0));
	sh_vertices.push_back(sh_vertices[0]);
	sh_vertices.push_back(sh_vertices[2]);

	sh_colors.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
	sh_colors.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
	sh_colors.push_back(glm::vec3(0.0f, 0.0f, 1.0f));

	glGenBuffers(1, &vertex_coordinate_handle);
	assert(vertex_coordinate_handle != (GLuint)-1);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_coordinate_handle);
	glBufferData(GL_ARRAY_BUFFER, sh_vertices.size() * sizeof(glm::vec2), &sh_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &color_value_handle);
	assert(color_value_handle != -1);
	glBindBuffer(GL_ARRAY_BUFFER, color_value_handle);
	glBufferData(GL_ARRAY_BUFFER, sh_colors.size() * sizeof(glm::vec3), &sh_colors[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &vertex_array_handle);
	glBindVertexArray(vertex_array_handle);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_coordinate_handle);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
	glBindBuffer(GL_ARRAY_BUFFER, color_value_handle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLReturnedError("InitializeGeometry - on exit");
	return true;
}
int main(int argc, char * argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(1024, 512);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	
	window_handle = glutCreateWindow("A More Sophisticated Modern Hello World");
	
	glutReshapeFunc(ReshapeFunc);
	glutCloseFunc(CloseFunc);
	glutDisplayFunc(DisplayFunc);
	glutKeyboardFunc(KeyboardFunc);
	glutTimerFunc(1000 / 60, TimerFunc, 1000 / 60);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	if (glewInit() != GLEW_OK)
	{
		cerr << "GLEW failed to initialize." << endl;
		cin.get();
		return 0;
	}

	if (!simple_shader.Initialize("simple_vertex_shader.glsl", "simple_fragment_shader.glsl"))
	{
		cerr << "simple_shader failed to initialize.\n";
		cin.get();
		return 0;
	}

	if (!InitializeGeometry())
	{
		cerr << "Geometry failed to initialize.\n";
		cin.get();
		return 0;
	}

	const int count_of_triangles = 50;

	MakeRandomPositions(count_of_triangles);
	MakeRandomOffsets(count_of_triangles);
	MakeRandomScalingFactors(count_of_triangles);

	glutMainLoop();

	simple_shader.TakeDown();
	cout << "Hit enter to exit:";
	cin.get();
	return 0;
}
