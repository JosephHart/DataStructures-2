//List of includes, system files and user created files
#include <gl/glut.h>
#include "app.h"
#include "coreMath.h"
#include "pcontacts.h"
#include "pworld.h"
#include "collision.h"
#include <stdio.h>
#include <cassert>
#include <random>
#include <time.h>

//Number of Particles, main system control of the amount of particles generated. 
//Advised to keep to 200 or less unless mass controls in the constructor are changed
const int NoOfParticles = 100;
//Gravity set to standard level
const Vector2 Vector2::GRAVITY = Vector2(0,-9.81);

/**
 * Platforms are two dimensional: lines on which the 
 * particles can rest. Platforms are also contact generators for the physics.
 */

class Platform : public ParticleContactGenerator
{
public:
    Vector2 start;
    Vector2 end;
    /**
     * Holds a pointer to the particles we're checking for collisions with. 
     */
    Particle *particle;

    virtual unsigned addContact(
        ParticleContact *contact, 
        unsigned limit
        ) const;
};

unsigned Platform::addContact(ParticleContact *contact, unsigned limit) const
{
    
	//const static float restitution = 0.8f;
	const static float restitution = 1.0f;
	unsigned used = 0;
        
        // Check for penetration
        Vector2 toParticle = particle->getPosition() - start;
        Vector2 lineDirection = end - start;

        float projected = toParticle * lineDirection;
        float platformSqLength = lineDirection.squareMagnitude();
		float squareRadius = particle->getRadius()*particle->getRadius();;
		 
       if (projected <= 0)
        {
		
            // The blob is nearest to the start point
            if (toParticle.squareMagnitude() < squareRadius)
            {
                // We have a collision
                contact->contactNormal = toParticle.unit();
                contact->restitution = restitution;
                contact->particle[0] = particle;
                contact->particle[1] = 0;
                contact->penetration = particle->getRadius() - toParticle.magnitude();
                used ++;
                contact ++;
            }
            
        }
        else if (projected >= platformSqLength)
        {
            // The blob is nearest to the end point
            toParticle = particle->getPosition() - end;
            if (toParticle.squareMagnitude() < squareRadius)
		    {
                // We have a collision
                contact->contactNormal = toParticle.unit();
                contact->restitution = restitution;
                contact->particle[0] = particle;
                contact->particle[1] = 0;
                contact->penetration = particle->getRadius() - toParticle.magnitude();
                used ++;            
                contact ++;
            }
        }
        else
        {
            // the blob is nearest to the middle.
            float distanceToPlatform = toParticle.squareMagnitude() - projected*projected / platformSqLength;
            if (distanceToPlatform < squareRadius)
            {
                // We have a collision
                Vector2 closestPoint = start + lineDirection*(projected/platformSqLength);

                contact->contactNormal = (particle->getPosition()-closestPoint).unit();
				contact->restitution = restitution;
                contact->particle[0] = particle;
                contact->particle[1] = 0;
				contact->penetration = particle->getRadius() - sqrt(distanceToPlatform);
	            used ++;
                contact ++;
            }
        }

    return used;
}

//Main class for application, overrides application class
class BlobDemo : public Application
{
	//Array of particles, control of array size at top of program
    Particle *blob[NoOfParticles];
    Platform *platform;
    ParticleWorld world;

public:
    /** Creates a new demo object. */
    BlobDemo();
    virtual ~BlobDemo();

    /** Returns the window title for the demo. */
    virtual const char* getTitle();

    /** Display the particles. */
    virtual void display();

    /** Update the particle positions. */
    virtual void update();

	//Collision detection methods
	//Checks to see if particles are close enough for a collision with the program window
	//Bounces particles off the sides of the window
	void box_collision_resolve(Particle &particle);
	//Checks to see if particles are outside of the window
	bool out_of_box_test(Particle particle);
	//Moves particles back in window if needed
	void out_of_box_resolve(Particle &particle);
	//Checks particles for validity for collision and then processes collisions by sending events to the collision class
	void particle_collision_test(Particle &particle);
};

// Method definitions
BlobDemo::BlobDemo():world(2, 1)
{
	//Global control for the window width and height
	width = 800; height = 800;
	//Range in units for the window, both positive and negative
	nRange = 100.0;		//Default = 100.0, giving range of 200 by 200 (-100 to 100)

	//Seeds random numbers at this point, needed to randomly geenrate particles
	srand(time(NULL));

    // Create the blobs for the program, core constructor loop for blob details
	for (int i = 0; i < NoOfParticles; i++)
	{
		//Create new particle
		blob[i] = new Particle;
		//Initiate random position of a particle within the nRange bounds specified further above
		//Changes to this area should be accomplished via changes to the nRange variable
		float randomX = -nRange + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (nRange - -nRange)));
		float randomY = -nRange + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (nRange - -nRange)));

		//Setup initial position and velocity of the particles
		blob[i]->setPosition(randomX, randomY);
		blob[i]->setVelocity(10, 10);

		//Set up damping for the particles or in other words the amount of energy particles lose on contact with other particles or the sides of the box
		//1.0 is default, which equates to no damping, lower numbers give more damping effects
		//blob[i]->setDamping(0.4);
		blob[i]->setDamping(1.0);

		//Acceleration force is initially set to that of gravity to pull particles down
		blob[i]->setAcceleration(Vector2::GRAVITY * 20.0f);

		//Main control for the upper and lower limits of mass
		//Changing these values will effect the colour of particles, the radius of particles and the mass of particles
		//Default is 1.0f to 10.0f, which gives blue as heavy larger particles and red as smaller faster particles
		float lowerMass = 1.0f;
		float upperMass = 10.0f;
		float randomMass = lowerMass + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (upperMass - lowerMass)));
		blob[i]->setMass(randomMass);
		blob[i]->setRed(1 / randomMass);
		blob[i]->setBlue(randomMass / upperMass);		//inverse calculation used to promote high blue in heavier objects
		blob[i]->setRadius(randomMass / 2);

		//Clear forces before program start
		blob[i]->clearAccumulator();
		//Set ID equal to loop counter
		//This is used in order to help speed up calculation checks between particles
		blob[i]->setID(i);
		//Collision status is set to false so that the particle defaults to not being in a collision state on program launch
		//variable is used for preventing multiple collisions by one particle in a single frame
		blob[i]->setCollisionStatus(false);
		//Particle world is assigned each particle in turn, currently unused
		world.getParticles().push_back(blob[i]);
	}
	   
    // Create and initialise the platform
	platform = new Platform;
	
	//platform->start = Vector2 ( -50.0, 0.0 );
	//platform->end   = Vector2 (  50.0, 0.0 );

    // Make sure the platform knows which particle it should collide with.
   // platform->particle = blob;

    //world.getContactGenerators().push_back(platform);
}

//Destructor for the demo
BlobDemo::~BlobDemo()
{
    delete blob;
}

//Main display function, used to render objects onto the screen
void BlobDemo::display()
{
	//Call main display function from the application class
	Application::display();

	//Defines the beginning and end of the platform
	const Vector2 &p0 = platform->start;
	const Vector2 &p1 = platform->end;

	//Renders the platform onto screen
	glBegin(GL_LINES);
	glColor3f(0,1,1);
	glVertex2f(p0.x, p0.y);
	glVertex2f(p1.x, p1.y);
	glEnd();

	//Begin main render loop for the particles
	for (int i = 0; i < NoOfParticles; i++)
	{
		//Sets the colour of the particles depending on the random values assigned in the constructor
		glColor3f(blob[i]->getRed(), blob[i]->getGreen(), blob[i]->getBlue());

		//Code block that renders the particles to the position they are in
		const Vector2 &p = blob[i]->getPosition();
		glPushMatrix();
		glTranslatef(p.x, p.y, 0);
		glutSolidSphere(blob[i]->getRadius(), 12, 12);	//Radius, slices of the circle, stacks of the circle
		glPopMatrix();
		//Push/Pop matrix is used to place a new matrix onto the stack and then removes it, in effect allows the translation of objects
	}

	//Presents the back buffer to the screen
	glutSwapBuffers();
}

//Main program update step
void BlobDemo::update()
{
    // Recenter the axes
	float duration = timeinterval/1000;
    // Run the simulation
    world.runPhysics(duration);

	//Main loop for all collisions
	for (int i = 0; i < NoOfParticles; i++)
	{
		//Checks to see if out of bounds or particle hits the edge of the box
		box_collision_resolve(*blob[i]);
		if (out_of_box_test(*blob[i])) out_of_box_resolve(*blob[i]);
		//Checks to see if particles collide with each other, if so feeds into the main algorithm for collision detection
		particle_collision_test(*blob[i]);
	}
	//Reset loop for reseting the state of collisions to false at the end of the frame update
	//This is needed because particles if they collide with each other have their status set to true so multiple collisions do not happen with the same particle in the same frame
	for (int i = 0; i < NoOfParticles; i++)
	{
		//Flicks bool to false
		blob[i]->setCollisionStatus(false);
	}

	//Run main application update step (does little)
    Application::update();
}

//Sets title for the program
const char* BlobDemo::getTitle()
{
    return "Collision Detection";
}

/**
 * Called by the common demo framework to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
    return new BlobDemo();
}

// detect if the particle colided with the box and produce a response
void BlobDemo::box_collision_resolve(Particle &particle)
{
	//Gets core variables for calculations
	Vector2 position = particle.getPosition();
	Vector2 velocity = particle.getVelocity();
	float radius = particle.getRadius();

	float w = Application::width;
	float h = Application::height;

	// Reverse direction when you reach left or right edge
	if (position.x> w - radius || position.x < -w + radius)
		particle.setVelocity(-velocity.x, velocity.y);

	if (position.y > h - radius || position.y < -h + radius)
		particle.setVelocity(velocity.x, -velocity.y);
}

//  Check bounds. This is in case the window is made
//  smaller while the sphere is bouncing and the 
//  sphere suddenly finds itself outside the new
//  clipping volume
bool BlobDemo::out_of_box_test(Particle particle)
{
	//Gets variables for testing if out of bounds
	Vector2 position = particle.getPosition();
	Vector2 velocity = particle.getVelocity();
	float radius = particle.getRadius();
	//Tests if out of bounds, returns true if so
	if ((position.x > Application::width - radius) || (position.x < -Application::width + radius)) return true;
	if ((position.y > Application::height - radius) || (position.y < -Application::height + radius)) return true;

	//false indicates particle within bounds
	return false;
}

//  Check bounds. This is in case the window is made
//  smaller while the sphere is bouncing and the 
//  sphere suddenly finds itself outside the new
//  clipping volume
void BlobDemo::out_of_box_resolve(Particle &particle)
{
	//Get variables for the resolution
	Vector2 position = particle.getPosition();
	Vector2 velocity = particle.getVelocity();
	float radius = particle.getRadius();

	//Sets up variables for moving a particle back to the box
	if (position.x > Application::width - radius)        position.x = Application::width - radius;
	else if (position.x < -Application::width + radius)  position.x = -Application::width + radius;

	if (position.y > Application::height - radius)        position.y = Application::height - radius;
	else if (position.y < -Application::height + radius)  position.y = -Application::height + radius;

	//Moves the particle to within the box if needed
	particle.setPosition(position.x, position.y);
}

//Main particle checking method, feeds into the collision.cpp files
void BlobDemo::particle_collision_test(Particle &particle)
{
	//Main loop for the particle array
	for (int i = 0; i < NoOfParticles; i++)
	{
		//Check to see if particle being checked against is the same as the particle we're checking
		//Also checks to see if a collision with each particle examined has happened this frame, if so then ignore said particle
		if (particle.getID() == blob[i]->getID() || particle.getCollisionStatus() == true || blob[i]->getCollisionStatus() == true)
		{
			return;
			//Do nothing if same particle
			//or if particle has been collided this frame
		}
		else
		{
			//Setup new collision
			Collision *collision = new Collision(&particle, blob[i]);

			//Check for collision
			//If collision is checked as true, then resolve the collision
			if(collision->checkForCollision()) collision->resolveCollision();

			//clean up
			delete collision;
		}
	}
}