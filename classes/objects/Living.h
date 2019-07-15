#ifndef livingH
#define livingH

#define LEVEL_EXP_VALUE 50

#include "PhysicQuad.h"

class MLiving: public MPhysicQuad {
protected:
	int Level;
	int Health;
	int Expirience;
	int NextLevelExpirience;
public:
	MLiving();
	void ReciveDamage(int Value);
	void SetHealth(int Value);
	int GetHealth();
	void SetExpirience(int Value);
	int GetExpirience();
	void AddExpirience(int Value);
	void LevelUp();
};

#endif
