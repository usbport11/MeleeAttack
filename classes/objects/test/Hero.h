#ifndef heroH
#define heroH

#include "Living.h"
#include "Enemy.h"
#include "Weapon.h"

class MHero: public MLiving {
private:
	bool NearLevelExitPrev;
	bool NearLevelExitNext;
	vector<MEnemy*> ContactEnemies;//list of contacting enemies
	MWeapon* pCurrentWeapon;
	b2RevoluteJoint* WeaponJoint;
protected:
	void OnBeginCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type);
	void OnEndCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type);
	void OnBeginWallCollide();
	void OnEndWallCollide();
public:
	MHero();
	bool GetNearLevelExitPrev();
	bool GetNearLevelExitNext();
	void ProcessContactEnemies();
	void EquipWeapon(MWeapon* pWeapon);
	void UnequipWeapon();
	void Attack(glm::vec2 MousePoint);
	MWeapon* GetCurrentWeapon();
	void Update();
};

#endif