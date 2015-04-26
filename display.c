
#include <stdio.h>
#include <GL/glut.h>

#include "v4l_base.h"
#include "cl_base.h"
#include "preprocess.h"
#include "fast.h"

GLuint tex = 0;
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
	glEnable(GL_POINT_SMOOTH);

	/* Set viewpoint */
	glOrtho(0.0, 640, 0.0, 480, -1.0, 1.0);
	glMatrixMode(GL_PROJECTION);
}

void renderScene(void)
{
	int i;
	char* dat;
	struct points pts = {0};
	struct v4l2_buffer* img;

	/* Read raw image data */
	img = v4l_base_dequeue(&v4l);

	/* Perform image preprocessing */
	dat = preprocess_run(&cl, &ts_preprocess, img->m.userptr);

	/* Add buffer back to the queue*/
	v4l_base_enqueue(&v4l, img);

	/* Run keypoint detection */
	pts = fast_run(&cl, &ts_fast, dat);

	/* Clear image buffer */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Set texture */
	glColor3f(1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 640, 480, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, dat);

	/* Draw quad*/
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 0);
		glTexCoord2f(1.0f, 1.0f); glVertex2f(640, 0);
		glTexCoord2f(1.0f, 0.0f); glVertex2f(640, 480);
		glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 480);
	glEnd();

	/* Unbind the texture */
	glBindTexture(GL_TEXTURE_2D, 0); 

	/* Draw points */
	glColor3f(1.0f,0.0f,0.0f);
	glBegin(GL_POINTS);
		for(i=0; i < pts.count; ++i)
		{
			glVertex2f(pts.coords[i].x, 480 - pts.coords[i].y);
		}
	glEnd();

	/* Swap image */
    glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char **argv) 
{
	int i;

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

	/* Add buffers */
	for(i = 0; i < 4; ++ i)
	{
		int size = sizeof(short) * 640 * 480;
		void* buf = malloc(size);
		v4l_base_mem_add(&v4l, buf, size);
	}

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