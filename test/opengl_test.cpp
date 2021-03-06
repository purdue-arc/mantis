

#include <ros/ros.h>

/* Copyright (c) Mark J. Kilgard, 1997. */

/* This program is freely distributable without licensing fees
   and is provided without guarantee or warrantee expressed or
   implied. This program is -not- in the public domain. */

/* This program was requested by Patrick Earl; hopefully someone else
   will write the equivalent Direct3D immediate mode program. */


#include <GL/glew.h>
#include <GL/glut.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/core.hpp"
#include <opencv2/core/opengl.hpp>
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/video.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void init();
void display();
void prepare();

int const fbo_width = 512;
int const fbo_height = 512;

GLuint fb, color, depth;



int main(int argc, char *argv[])
{

	ros::init(argc, argv, "unit_test");
	char *myargv [1];
	int myargc=1;
	myargv [0]=strdup ("Myappname");
	glutInit(&myargc, myargv);
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutCreateWindow("FBO test");
	//glutDisplayFunc(display);
	//glutIdleFunc(glutPostRedisplay);
	glutHideWindow();

	glewInit();

	ROS_DEBUG("here0");

	init();
	//glutMainLoop();
	while(ros::ok())
	{
		prepare();
	}

	return 0;
}

void CHECK_FRAMEBUFFER_STATUS()
{
	GLenum status;
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch(status) {
	case GL_FRAMEBUFFER_COMPLETE:
		break;

	case GL_FRAMEBUFFER_UNSUPPORTED:
		/* choose different formats */
		break;

	default:
		/* programming error; will fail on all hardware */
		fputs("Framebuffer Error\n", stderr);
		exit(-1);
	}
}

float const light_dir[]={1,1,1,0};
float const light_color[]={1,0.95,0.9,1};

void init()
{

	glGenFramebuffers(1, &fb);
	glGenTextures(1, &color);
	glGenRenderbuffers(1, &depth);

	ROS_DEBUG("here1.25");

	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	ROS_DEBUG("here1.5");
	glBindTexture(GL_TEXTURE_2D, color);
	glTexImage2D(   GL_TEXTURE_2D,
			0,
			GL_RGBA,
			fbo_width, fbo_height,
			0,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			NULL);

	ROS_DEBUG("here1");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo_width, fbo_height);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

	CHECK_FRAMEBUFFER_STATUS();
}

void prepare()
{
	static float a=0, b=0, c=0;

	ROS_DEBUG("start");
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_TEXTURE_2D);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glViewport(0,0, fbo_width, fbo_height);

	glClearColor(1,1,1,0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1, 1, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glLightfv(GL_LIGHT0, GL_POSITION, light_dir);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_color);

	glTranslatef(0,0,-5);

	glRotatef(a, 1, 0, 0);
	glRotatef(b, 0, 1, 0);
	glRotatef(c, 0, 0, 1);
ROS_DEBUG("before tp");
	glutSolidTeapot(0.75);
	ROS_DEBUG("after tp");

	a=fmod(a+0.1, 360.);
	b=fmod(b+0.5, 360.);
	c=fmod(c+0.25, 360.);


	cv::Mat temp = cv::Mat(fbo_height, fbo_width, CV_8UC3);
	cv::Mat result;

	//glutSwapBuffers();
	glReadBuffer(GL_BACK);

	//glPixelStorei(GL_PACK_ALIGNMENT, (temp.step & 3) ? 1 : 4);
	glPixelStorei(GL_PACK_ROW_LENGTH, temp.step/temp.elemSize());
	// Read Image from buffer
	glReadPixels(0,0, temp.cols, temp.rows, GL_BGR, GL_UNSIGNED_BYTE,temp.data);

	// Process buffer so it matches correct format and orientation
	//cv::cvtColor(temp, tempImage, CV_BGR2RGB);
	cv::flip(temp, result, 0);


	cv::imshow("test",result);
	cv::waitKey(30);
	ROS_DEBUG("end");

	glutHideWindow();
}

void final()
{
	static float a=0, b=0, c=0;

	const int win_width  = glutGet(GLUT_WINDOW_WIDTH);
	const int win_height = glutGet(GLUT_WINDOW_HEIGHT);
	const float aspect = (float)win_width/(float)win_height;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0,0, win_width, win_height);

	glClearColor(1.,1.,1.,0.);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, aspect, 1, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0,0,-5);

	glRotatef(b, 0, 1, 0);

	b=fmod(b+0.5, 360.);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, color);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_LIGHTING);

	float cube[][5]=
	{
			{-1, -1, -1,  0,  0},
			{ 1, -1, -1,  1,  0},
			{ 1,  1, -1,  1,  1},
			{-1,  1, -1,  0,  1},

			{-1, -1,  1, -1,  0},
			{ 1, -1,  1,  0,  0},
			{ 1,  1,  1,  0,  1},
			{-1,  1,  1, -1,  1},
	};
	unsigned int faces[]=
	{
			0, 1, 2, 3,
			1, 5, 6, 2,
			5, 4, 7, 6,
			4, 0, 3, 7,
			3, 2, 6, 7,
			4, 5, 1, 0
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 5*sizeof(float), &cube[0][0]);
	glTexCoordPointer(2, GL_FLOAT, 5*sizeof(float), &cube[0][3]);

	glCullFace(GL_BACK);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, faces);

	glCullFace(GL_FRONT);
	glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, faces);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void display()
{
	prepare();
	//final();

	//glutSwapBuffers();
}
