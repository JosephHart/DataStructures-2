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
#include <random>
#include <time.h>

//Number of Particles
const int NoOfParticles = 20;
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

	void box_collision_resolve(Particle &particle);
	bool out_of_box_test(Particle particle);
	void out_of_box_resolve(Particle &particle);
	void particle_collision_test(Particle &particle);
	void particle_collision_resolve(Particle &particle1, Particle &particle2);
	
};

// Method definitions
BlobDemo::BlobDemo():world(2, 1)
{
	width = 800; height = 800;
	nRange = 100.0;
	srand(time(NULL));

    // Create the blob storage
	for (int i = 0; i < NoOfParticles; i++)
	{
		blob[i] = new Particle;
		float randomX = -nRange + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (nRange - -nRange)));
		float randomY = -nRange + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (nRange - -nRange)));

		// Create the blob
		blob[i]->setPosition(randomX, randomY);
		blob[i]->setVelocity(10, 10);
		//blob[i]->setDamping(0.9);
		blob[i]->setDamping(1.0);
		blob[i]->setAcceleration(Vector2::GRAVITY * 20.0f);

		float lowerMass = 1.0f;
		float upperMass = 10.0f;
		float randomMass = lowerMass + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (upperMass - lowerMass)));
		blob[i]->setMass(randomMass);
		blob[i]->setRed(1 / randomMass);
		blob[i]->setBlue(randomMass / upperMass);
		blob[i]->setRadius(randomMass / 2);

		blob[i]->clearAccumulator();
		blob[i]->setID(i);
		blob[i]->setCollisionStatus(false);
		world.getParticles().push_back(blob[i]);
	}
	   
    // Create the platform
	platform = new Platform;
	
	//platform->start = Vector2 ( -50.0, 0.0 );
	//platform->end   = Vector2 (  50.0, 0.0 );

    // Make sure the platform knows which particle it should collide with.
   // platform->particle = blob;

    //world.getContactGenerators().push_back(platform);


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

   for (int i = 0; i < NoOfParticles; i++)
   {
	   glColor3f(blob[i]->getRed(), blob[i]->getGreen(), blob[i]->getBlue());

	   const Vector2 &p = blob[i]->getPosition();
	   glPushMatrix();
	   glTranslatef(p.x, p.y, 0);
	   glutSolidSphere(blob[i]->getRadius(), 12, 12);
	   glPopMatrix();
   }

	glutSwapBuffers();
    
}

void BlobDemo::update()
{
    // Recenter the axes
	float duration = timeinterval/1000;
    // Run the simulation
    world.runPhysics(duration);

	for (int i = 0; i < NoOfParticles; i++)
	{
		box_collision_resolve(*blob[i]);
		if (out_of_box_test(*blob[i])) out_of_box_resolve(*blob[i]);
		particle_collision_test(*blob[i]);
	}
	for (int i = 0; i < NoOfParticles; i++)
	{
		blob[i]->setCollisionStatus(false);
	}

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

// detect if the particle colided with the box and produce a response
void BlobDemo::box_collision_resolve(Particle &particle)
{
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
	Vector2 position = particle.getPosition();
	Vector2 velocity = particle.getVelocity();
	float radius = particle.getRadius();
	if ((position.x > Application::width - radius) || (position.x < -Application::width + radius)) return true;
	if ((position.y > Application::height - radius) || (position.y < -Application::height + radius)) return true;

	return false;
}

//  Check bounds. This is in case the window is made
//  smaller while the sphere is bouncing and the 
//  sphere suddenly finds itself outside the new
//  clipping volume
void BlobDemo::out_of_box_resolve(Particle &particle)
{
	Vector2 position = particle.getPosition();
	Vector2 velocity = particle.getVelocity();
	float radius = particle.getRadius();


	if (position.x > Application::width - radius)        position.x = Application::width - radius;
	else if (position.x < -Application::width + radius)  position.x = -Application::width + radius;

	if (position.y > Application::height - radius)        position.y = Application::height - radius;
	else if (position.y < -Application::height + radius)  position.y = -Application::height + radius;

	particle.setPosition(position.x, position.y);
}

void BlobDemo::particle_collision_test(Particle &particle)
{
	for (int i = 0; i < NoOfParticles; i++)
	{
		if (particle.getID() == blob[i]->getID() || particle.getCollisionStatus() == true || blob[i]->getCollisionStatus() == true)
		{
			//Do nothing if same particle
			int test = 0;
		}
		else
		{
			Vector2 vecD = particle.getPosition() - blob[i]->getPosition();
			float distance = sqrt(pow(vecD.x, 2) + pow(vecD.y, 2));

			float sumRadius = particle.getRadius() + blob[i]->getRadius();
			if (distance <= sumRadius)
			{
				Vector2 interpenetrationVec = particle.getPosition() - blob[i]->getPosition();

				float penetration = (particle.getRadius() + blob[i]->getRadius()) - interpenetrationVec.magnitude();

				interpenetrationVec.normalise();
				interpenetrationVec *= penetration;

				particle.setPosition(particle.getPosition() + interpenetrationVec * 0.5);
				blob[i]->setPosition(blob[i]->getPosition() - interpenetrationVec * 0.5);

				particle_collision_resolve(particle, *blob[i]);
			}
			//else no collision
		}
	}
}

void BlobDemo::particle_collision_resolve(Particle &particle1, Particle &particle2)
{
	Vector2 x = particle1.getPosition() - particle2.getPosition();
	x.normalise();

	//Sphere1
	Vector2 v1 = particle1.getVelocity();
	float x1 = x.scalarProduct(v1);
	Vector2 v1x = x * x1;
	Vector2 v1y = v1 - v1x;
	float m1 = particle1.getMass();

	//Sphere2
	x = x * -1;
	Vector2 v2 = particle2.getVelocity();
	float x2 = x.scalarProduct(v2);
	Vector2 v2x = x * x2;
	Vector2 v2y = v2 - v2x;
	float m2 = particle2.getMass();

	//Calculate final velocities
	particle1.setVelocity(v1x * ((m1 - m2) / (m1 + m2)) + v2x * ((2 * m2) / (m1 + m2)) + v1y);
	particle2.setVelocity(v1x * ((2 * m1) / (m1 + m2)) + v2x * ((m2 - m1) / (m1 + m2)) + v2y);

	particle1.setCollisionStatus(true);
	particle2.setCollisionStatus(true);
}