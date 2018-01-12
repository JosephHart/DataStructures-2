#pragma once
#include "particle.h"

//Collision class declaration
//This class deals with the collisions between particles
class Collision
{
	//Particle variables stored so that collision detection can be calculated between the two
	//Private by default
	Particle *particle1;
	Particle *particle2;

public:
	//Declaration of methods for the collision class
	//Default constructor and checking/resolving methods
	Collision(Particle *p1, Particle *p2);
	bool checkForCollision();
	void resolveCollision();
};