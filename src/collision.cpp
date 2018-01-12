#include "collision.h"

//Default constructor the collision, assigns two particles for a collision to the object's variables for future calculations
Collision::Collision(Particle *p1, Particle *p2)
{
	particle1 = p1;
	particle2 = p2;
}

//Method that checks for a collision, if a collision is found it returns true
bool Collision::checkForCollision()
{
	//Distance calculations and checks for determining if a collision is possible
	Vector2 vecD = particle1->getPosition() - particle2->getPosition();
	float distance = sqrt(pow(vecD.x, 2) + pow(vecD.y, 2));
	float sumRadius = particle1->getRadius() + particle2->getRadius();
	//If the distance is smaller than the sum of both particles radius's then a collision will occur
	if (distance <= sumRadius)
	{
		//Fluffing code
		//Moves particle slightly off centre from the collision if particles overlap
		//In essence this section deals with the issue of particles wrapping around each other or circling each other
		Vector2 interpenetrationVec = particle1->getPosition() - particle2->getPosition();
		float penetration = (particle1->getRadius() + particle2->getRadius()) - interpenetrationVec.magnitude();
		interpenetrationVec.normalise();
		interpenetrationVec *= penetration;
		particle1->setPosition(particle1->getPosition() + interpenetrationVec * 0.5);
		particle2->setPosition(particle2->getPosition() - interpenetrationVec * 0.5);
		//Without this section of code, particles will circle each other in death spirals every so often

		//since the distance is closer than the sum of radius's, a collision will occur and the method will return true
		return true;
	}
	//if distance is greater then no collision will occur
	return false;
}

//Particle collision method that performs all calculations and variable changes for a collision
void Collision::resolveCollision()
{
	//Initial vector of the difference in position is taken
	Vector2 x = particle1->getPosition() - particle2->getPosition();
	//Vector is normalised to find common plane between particles
	x.normalise();

	//Sphere1 Calculations
	Vector2 v1 = particle1->getVelocity();
	float x1 = x.scalarProduct(v1);
	Vector2 v1x = x * x1;
	Vector2 v1y = v1 - v1x;
	float m1 = particle1->getMass();

	//Sphere2 Calculations
	x = x * -1;
	Vector2 v2 = particle2->getVelocity();
	float x2 = x.scalarProduct(v2);
	Vector2 v2x = x * x2;
	Vector2 v2y = v2 - v2x;
	float m2 = particle2->getMass();

	//Calculate final velocities
	//This is based on newtonian physics
	//Worth stating I don't understand all the mathematics in this section of code, both above and below
	//But I do know how to use and implement it
	particle1->setVelocity(v1x * ((m1 - m2) / (m1 + m2)) + v2x * ((2 * m2) / (m1 + m2)) + v1y);
	particle2->setVelocity(v1x * ((2 * m1) / (m1 + m2)) + v2x * ((m2 - m1) / (m1 + m2)) + v2y);

	//Sets the collision status of particles to true for this frame
	//This is used to prevent multiple collisions occuring for one particle in a single frame of the application
	particle1->setCollisionStatus(true);
	particle2->setCollisionStatus(true);
}