#include "../../stdafx.h"
#include "Living.h"
#include "Weapon.h"

MWeapon::MWeapon():MPhysicQuad() {
	Damage[0] = Damage[1] = 0;
	CanRestart = true;
	InProcess = false;
	StopByWall = false;
}

void MWeapon::GetDamage(int& MinValue, int& MaxValue) {
	MinValue = Damage[0];
	MaxValue = Damage[1];
}

void MWeapon::SetDamage(int MinValue, int MaxValue) {
	Damage[0] = MinValue;
	Damage[1] = MaxValue;
}

bool MWeapon::GetCanRestart() {
	return CanRestart;
}

void MWeapon::SetCanRestart(bool Value) {
	CanRestart = Value;
}

void MWeapon::BeginAttack() {
	if(!CanRestart && InProcess) return;
	DamagedQuads.clear();
	InProcess = true;
}

void MWeapon::EndAttack() {
	InProcess = false;
	DamagedQuads.clear();
}

void MWeapon::OnBeginCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type) {
	if((Type == OT_HERO || Type == OT_ENBODY) && InProcess) {
		vector<MPhysicQuad*>::iterator it;
		it = find(DamagedQuads.begin(), DamagedQuads.end(), pPhysicQuad);
		if(it == DamagedQuads.end()) {
			MLiving* pLiving = (MLiving*)pPhysicQuad;
			pLiving->ReciveDamage(RandomValue(Damage[0], Damage[1]));//different damage to different quads?
			DamagedQuads.push_back(pPhysicQuad);
		}
	}
}

void MWeapon::OnEndCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type) {
}

void MWeapon::OnBeginWallCollide() {
	if(StopByWall) EndAttack();
}

void MWeapon::OnEndWallCollide() {
}
