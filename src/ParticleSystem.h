//  Kevin M. Smith - CS 134 SJSU

#include "ofMain.h"
#include "Particle.h"


//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
	bool applyOnce = false;
	bool applied = false;
	virtual void updateForce(Particle *) = 0;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
	void remove(int);
	void update();
	void setLifespan(float);
	void reset();
	int removeNear(const ofVec3f & point, float dist);
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
};



// Some convenient built-in forces
//
class GravityForce : public ParticleForce {
	ofVec3f gravity;
public:
	GravityForce(const ofVec3f & gravity);
	void set(const ofVec3f &g);
	void updateForce(Particle *);
};

class TurbulenceForce : public ParticleForce {
	ofVec3f tmin, tmax;
public:
	TurbulenceForce(const ofVec3f & min, const ofVec3f &max);
	void set(const ofVec3f &min, const ofVec3f &max);
	void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
	float magnitude, height;
public:
	ImpulseRadialForce(float m);
	void setHeight(float h);
	void updateForce(Particle *);
};

class ThrustForce : public ParticleForce {
	ofVec3f direction;
public:
	ThrustForce(const ofVec3f &dir);
	void set(const ofVec3f &dir);
	void updateForce(Particle *);
};

class ImpulseForce : public ParticleForce {
public:
	ofVec3f force;
	ImpulseForce(const ofVec3f f);
	void set(const ofVec3f &dir);
	void apply(const ofVec3f f);
	void updateForce(Particle *particle);
};