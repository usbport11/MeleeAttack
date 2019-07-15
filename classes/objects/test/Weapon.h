#ifndef weaponH
#define weaponH

#include "PhysicQuad.h"

class MWeapon: public MPhysicQuad {
private:
	int Damage[2];//min/max values
	float AttackSpeed;
	float SwingAngle;
	float AttackAngle;//at 0 degrees []---
	vector<MPhysicQuad*> DamagedQuads;//list of damaged quads by one attack
protected:
	bool CanRestart;
	bool InProcess;
	bool StopByWall;
public:
	MWeapon();
	bool GetInProcess();
	float GetSwingAnle();
	void SetSwingAngle(float Value);
	void GetDamage(int& MinValue, int& MaxValue);
	void SetDamage(int MinValue, int MaxValue);
	bool GetCanRestart();
	void SetCanRestart(bool Value);
	void BeginAttack(float inAttackAngle);
	void EndAttack();
	void OnBeginCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type);
	void OnEndCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type);
	void OnBeginWallCollide();
	void OnEndWallCollide();
	void Update();
};

#endif
