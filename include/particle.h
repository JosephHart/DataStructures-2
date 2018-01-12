    /**
     * A particle is the simplest object that can be simulated in the
     * physics system.

     */

#ifndef PARTICLE_H
#define PARTICLE_H

#include "coreMath.h"

class Particle
{
protected:
//Particle variables used to control all aspects of a particle
//Self documenting
float inverseMass;
float damping;
float radius;
Vector2 position;
Vector2 velocity;
Vector2 forceAccum;
Vector2 acceleration;
int ID;
bool collisionStatus;
float red, green, blue;
    
public:
	//A long list of accessor and mutator methods, I do not need to comment on each of these
	//Code here is in my opinion, self documenting
	void integrate(float duration);
	void setMass(const float mass);
	float getMass() const;
	void setInverseMass(const float inverseMass);
	float getInverseMass() const;
	bool hasFiniteMass() const;

	void setDamping(const float damping);
    float getDamping() const;

    void setPosition(const float x, const float y);
	void setPosition(const Vector2 &position);
	Vector2 getPosition() const;
	void getPosition(Vector2 *position) const;
		
	void setRadius(const float r);
	float getRadius() const;
		
	void setVelocity(const Vector2 &velocity);
	void setVelocity(const float x, const float y);
	Vector2 getVelocity() const;
	void getVelocity(Vector2 *velocity) const;

	void setAcceleration(const Vector2 &acceleration);
	void setAcceleration(const float x, const float y);
	Vector2 getAcceleration() const;

	void clearAccumulator();
	void addForce(const Vector2 &force);

	int getID();
	void setID(int i);

	bool getCollisionStatus();
	void setCollisionStatus(bool c);

	float getRed();
	float getGreen();
	float getBlue();

	void setRed(float r);
	void setGreen(float g);
	void setBlue(float b);
    };

#endif // 