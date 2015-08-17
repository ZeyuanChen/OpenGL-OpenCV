/* partial code from http://www.alecjacobson.com/weblog/?p=1875
*/

// C++
#include <iostream>
#include <cstdio>
#include <ctime>
#include <map>
#include <time.h>
#include <thread>         // std::thread
#include <mutex>          // std::mutex




// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/video/tracking.hpp>

// Standard includes
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

using namespace cv;
using namespace std;

// all variables initialized to 1.0, meaning
// the triangle will initially be white
float _angle = 0.0;
float _cameraangle = 30.0;

// angle of rotation for the camera direction
float angle = 0.0;
// actual vector representing the camera's direction
float lx = 0.0f, lz = -1.0f;
// XZ position of the camera
float x = 0.0f, z = 5.0f;

// Opencv Parameters
int frame_width = 640;
int frame_height = 480;
int frame_channels = 3;


GLint g_hWindow;
CvCapture* g_Capture;
VideoCapture cap(0);
Mat frame, frame_vis;

GLint   windowWidth = 640;     // Define our window width
GLint   windowHeight = 480;     // Define our window height
GLfloat fieldOfView = 45.0f;   // FoV
GLfloat zNear = 0.1f;    // Near clip plane
GLfloat zFar = 200.0f;  // Far clip plane


// Frame counting and limiting
int    frameCount = 0;

GLuint matToTexture(cv::Mat &mat, GLenum minFilter, GLenum magFilter, GLenum wrapFilter)
{
	// Generate a number for our textureID's unique handle
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind to our texture handle
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Catch silly-mistake texture interpolation method for magnification
	if (magFilter == GL_LINEAR_MIPMAP_LINEAR ||
		magFilter == GL_LINEAR_MIPMAP_NEAREST ||
		magFilter == GL_NEAREST_MIPMAP_LINEAR ||
		magFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		cout << "You can't use MIPMAPs for magnification - setting filter to GL_LINEAR" << endl;
		magFilter = GL_LINEAR;
	}

	// Set texture interpolation methods for minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	// Set texture clamping method
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapFilter);

	// Set incoming texture format to:
	// GL_BGR       for CV_CAP_OPENNI_BGR_IMAGE,
	// GL_LUMINANCE for CV_CAP_OPENNI_DISPARITY_MAP,
	// Work out other mappings as required ( there's a list in comments in main() )
	GLenum inputColourFormat = GL_RGB;
	if (mat.channels() == 1)
	{
		inputColourFormat = GL_LUMINANCE;
	}

	// Create the texture
	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
		0,                 // Pyramid level (for mip-mapping) - 0 is the top level
		GL_RGB,            // Internal colour format to convert to
		mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
		mat.rows,          // Image height i.e. 480 for Kinect in standard mode
		0,                 // Border width in pixels (can either be 1 or 0)
		inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		GL_UNSIGNED_BYTE,  // Image data type
		mat.ptr());        // The actual image data itself

	

	return textureID;
}

void drawSnowMan() {

	glColor3f(1.0f, 1.0f, 1.0f);

	// Draw Body
	glTranslatef(0.0f, 0.75f, 0.0f);
	glutSolidSphere(0.75f, 20, 20);

	// Draw Head
	glTranslatef(0.0f, 1.0f, 0.0f);
	glutSolidSphere(0.25f, 20, 20);

	// Draw Eyes
	glPushMatrix();
	glColor3f(0.0f, 0.0f, 0.0f);
	glTranslatef(0.05f, 0.10f, 0.18f);
	glutSolidSphere(0.05f, 10, 10);
	glTranslatef(-0.1f, 0.0f, 0.0f);
	glutSolidSphere(0.05f, 10, 10);
	glPopMatrix();

	// Draw Nose
	glColor3f(1.0f, 0.5f, 0.5f);
	glutSolidCone(0.08f, 0.5f, 10, 2);
}


void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = w * 1.0 / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}





void update(int value)
{
	/*
	_angle += 2.0f;
	if (_angle>360.f)
	{
		_angle -= 360;
	}*/
	
	cap >> frame;

	//flip(frame, frame_vis, 1); //use the mirror frame
	frame_vis = frame.clone();    // refresh visualisation frame
	cvtColor(frame_vis, frame_vis, CV_BGR2RGB);
	
	// Create the texture
	/*
	glTexImage2D(GL_TEXTURE_2D,     // Type of texture
		0,                 // Pyramid level (for mip-mapping) - 0 is the top level
		GL_RGB,            // Internal colour format to convert to
		frame_vis.cols,          // Image width  i.e. 640 for Kinect in standard mode
		frame_vis.rows,          // Image height i.e. 480 for Kinect in standard mode
		0,                 // Border width in pixels (can either be 1 or 0)
		inputColourFormat, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
		GL_UNSIGNED_BYTE,  // Image data type
		frame_vis.ptr());        // The actual image data itself
*/

	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}

void renderScene(void) {

	GLenum inputColourFormat = GL_RGB;
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLuint textureID;
	glGenTextures(1, &textureID);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_vis.cols, frame_vis.rows, 0, GL_RGB, GL_UNSIGNED_BYTE, frame_vis.ptr());
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-1.0, 1.0, -2.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(1.0, 1.0, -2.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(1.0, -1.0, -2.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-1.0, -1.0, -2.0);
	glEnd();
	glDeleteTextures(1, &textureID);

	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0f, 0.0f, 10.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	GLfloat MaterialColor[] = { 0, 0, 0, 0 };
	GLfloat ambientColor[] = { 0.5, 0.0, 0.0, 0.0 };
	GLfloat specularColor[] = { 0.7, 0.6, 0.6, 0.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, ambientColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.25*128.0);
	glutSolidTeapot(1.0f);

	/*
	glEnable(GL_TEXTURE_2D);
	// These are necessary if using glTexImage2D instead of gluBuild2DMipmaps
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
*/
	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fieldOfView / 2.0f, windowWidth / windowHeight, zNear, zFar);*/
	// Switch to Model View Matrix
	/*
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Set the camera
	gluLookAt(0.0f, 0.0f, 10.0f,
		0.0f, 0.0f, 2.0f,
		0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	glVertex3f(-2.0f, -2.0f, 1.0f);
	glVertex3f(2.0f, 0.0f, 1.0);
	glVertex3f(0.0f, 2.0f, 1.0);
	glEnd();
	// Set Projection Matrix
	
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, frame_width, frame_height, 0);

	// Switch to Model View Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	

	
	// Draw a textured quad
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0, -1.0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0, -1.0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0, 1.0, -1.0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0, 1.0, -1.0);
	glEnd();
*/

	glFlush();
	glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y) {

	if (key == 27)
		exit(0);
}

void processSpecialKeys(int key, int xx, int yy) {

	float fraction = 0.1f;

	switch (key) {
	case GLUT_KEY_LEFT:
		angle -= 0.01f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	case GLUT_KEY_RIGHT:
		angle += 0.01f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	case GLUT_KEY_UP:
		x += lx * fraction;
		z += lz * fraction;
		break;
	case GLUT_KEY_DOWN:
		x -= lx * fraction;
		z -= lz * fraction;
		break;
	}
}

GLvoid init_glut()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	

	// register callbacks
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);
	glutTimerFunc(25, update, 0);

}


int main(int argc, char **argv) {

	//init camera
	/*
	CvCapture* g_Capture = 0;
	g_Capture = cvCaptureFromCAM(-1);
	assert(g_Capture);*/
	// capture properties
	if (!cap.isOpened())   // check if we succeeded
	{
		std::cout << "Could not open the camera device" << endl;
		return -1;
	}
	glutInit(&argc, argv);
	

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(frame_width, frame_height);
	g_hWindow = glutCreateWindow("Video Texture");
	// init GLUT and create window
	init_glut();

	
	
	// enter GLUT event processing cycle
	glutMainLoop();

	return 1;
}