#include <gl/glut.h>
#include "app.h"
//System files and application pointers used for referencing the app
extern Application* getApplication();
Application* app;

//Main display function, used to reference the application display function
//Note this is overriden by the blobdemo.cpp's display function but it's called anyway
void display(void)
{
	app->display();
}

//OpenGL setup code for a new window
void createWindow(const char* title, int h, int w)
{
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    glutCreateWindow(title);
}

//Used to regulate the speed of the program
void TimerFunc(int value)
{
    app->update();
	float  timeinterval = app->getTimeinterval();
	glutTimerFunc(timeinterval, TimerFunc, 1);
}

//Calls the resize function to resize the window to whatever the user drags it into
void resize(int width, int height)
{
    app->resize(width, height);
}

//Main function, program entrance and main execution area
int main(int argc, char* argv[])
{
	//A collection of glut and OpenGl functions that in essence run the graphical window
	//This section uses the various OpenGL and glut callback functions that are automatically cycled over and over for the application
	//The display function as one example will be called repeatedly until the main window is closed or the program terminates
    glutInit(&argc, argv);
    app = getApplication();
	float  timeinterval = 10;
	app->setTimeinterval(timeinterval);
	createWindow("Particle Collision Project", app->getheight(), app->getwidth());
	glutReshapeFunc(resize);
	glutDisplayFunc(display); 
	glutTimerFunc(timeinterval, TimerFunc, 1);
	app->initGraphics();
	glutMainLoop();
	delete app; 
	return 0;
}