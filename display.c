
#include <GL/glut.h>
#include "v4l_base.h"
#include "cl_base.h"
#include "preprocess.h"
#include "fast.h"

GLuint tex = 0;          /* OpenGL texture buffer */
struct v4l_base v4l;
struct cl_base cl;
struct cl_task ts_preprocess;
struct cl_task ts_fast;

void InitTexture()
{
	/* Create texture */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glEnable(GL_TEXTURE_2D);

	/* Set viewpoint */
	glOrtho(0.0, 640, 0.0, 480, -1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
}

void renderScene(void)
{
	struct v4l_img* img;

	/* Clear image buffer */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw a textured quad */
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, tex);

	/* Grab image data */
 	img = v4l_base_read(&v4l);
	char* dat = preprocess_run(&cl, &ts_preprocess, img->start);

	/* Set texture */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, dat);

	/* Draw quad*/
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 0);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(640, 0);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(640, 480);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 480);
	glEnd();

	/* Swap image */
    glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char **argv) 
{
	/* Init GLUT */
	glutInit(&argc, argv);

	/* Create window */
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(640, 480);
	glutCreateWindow("Hello World");
	InitTexture();

	/* Initialize devices and kernels */
	v4l_base_init(&v4l, "/dev/video0", 640, 480);
	cl_base_init(&cl);
	preprocess_init(&cl, &ts_preprocess);
	fast_init(&cl, &ts_fast, 0, 0, 0);
	v4l_base_capture_start(&v4l);

	/* Register callbacks */
	glutDisplayFunc(renderScene);

	/* Enter GLUT event processing cycle */
	glutMainLoop();
	
	return 1;
}