/*
 * The Blob demo.
 *
 */
#include <gl/glut.h>
#include "app.h"
#include "coreMath.h"
#include "pcontacts.h"
#include "pworld.h"
#include <stdio.h>
#include <cassert>


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

unsigned Platform::addContact(ParticleContact *contact, 
                              unsigned limit) const
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


class BlobDemo : public Application
{
    Particle *blob;

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
	
};

// Method definitions
BlobDemo::BlobDemo():world(2, 1)
{
	width = 400; height = 400; 
	nRange = 100.0;

    // Create the blob storage
    blob = new Particle;
	   
    // Create the platform
	platform = new Platform;
	
	platform->start = Vector2 ( -50.0, 0.0 );
	platform->end   = Vector2 (  50.0, 0.0 );

    // Make sure the platform knows which particle it should collide with.
    platform->particle = blob;

    world.getContactGenerators().push_back(platform);

    // Create the blob
	blob->setPosition(0.0, 90.0);
	blob->setRadius( 5 );
    blob->setVelocity(0,0); 
    //blob->setDamping(0.9);
	blob->setDamping(1.0);
    blob->setAcceleration(Vector2::GRAVITY * 20.0f ); 
    blob->setMass(30.0f);
    blob->clearAccumulator();
    world.getParticles().push_back(blob);
}


BlobDemo::~BlobDemo()
{
    delete blob;
}

void BlobDemo::display()
{
  Application::display();

  const Vector2 &p0 = platform->start;
  const Vector2 &p1 = platform->end;

   glBegin(GL_LINES);
   glColor3f(0,1,1);
   glVertex2f(p0.x, p0.y);
   glVertex2f(p1.x, p1.y);
   glEnd();
	
   glColor3f(1,0,0); 

    const Vector2 &p = blob->getPosition();
	glPushMatrix();
	glTranslatef(p.x, p.y, 0);
	glutSolidSphere(blob->getRadius(), 12, 12);
	glPopMatrix();

	glutSwapBuffers();
    
}

void BlobDemo::update()
{
    // Recenter the axes
	float duration = timeinterval/1000;
    // Run the simulation
    world.runPhysics(duration);

    Application::update();
}

const char* BlobDemo::getTitle()
{
    return "Blob Demo";
}

/**
 * Called by the common demo framework to create an application
 * object (with new) and return a pointer.
 */
Application* getApplication()
{
    return new BlobDemo();
}