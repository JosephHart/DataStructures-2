//Main system files
#include <gl/glut.h>
#include "app.h"

//Various basic getters/setters
int Application::getwidth() { return width; }
int Application::getheight() { return height; }

float Application::getTimeinterval() { return timeinterval; }
void Application::setTimeinterval(float timeinterval) { Application::timeinterval = timeinterval; }

//Sets the background colour of the screen to black, 1.0f is for the alpha channel which is entirely opaque
void Application::initGraphics() { glClearColor(0.0f, 0.0f, 0.0f, 1.0f ); }

//Display function that clears the graphical state
void Application::display() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//Glut's refresh function, used to update the screen when needed
void Application::update() { glutPostRedisplay(); }

//Resize function that transforms the size of the screen depending on how the user has moved the window
void Application::resize(int width, int height)
{
	// Prevent a divide by zero
	if (height == 0) height = 1;
    
	//nRange = 100.0f;	//Obsolete due to width and height being passed in, legacy
    GLfloat aspectRatio = (GLfloat)width / (GLfloat)height;

    // Set Viewport to window dimensions
    glViewport(0, 0, width, height);

	// Reset coordinate system
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

     //Establish clipping volume (left, right, bottom, top, near, far)
    if (width <= height) 
	{
		Application::width = nRange;
		Application::height = nRange/aspectRatio;
		glOrtho (-nRange, nRange, -nRange/aspectRatio, nRange/aspectRatio, -nRange*2.0f, nRange*2.0f);
	}
    else 
	{
		Application::width = nRange*aspectRatio;
		Application::height = nRange;
		glOrtho (-nRange*aspectRatio, nRange*aspectRatio, -nRange, nRange, -nRange*2.0f, nRange*2.0f);
	}

	// Reset the modelview matrix, clears matrix by loading identity
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}