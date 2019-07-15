#include "../../stdafx.h"
#include "Hero.h"

MHero::MHero():MLiving() {
	NearLevelExitPrev = false;
	NearLevelExitNext = false;
	pWeapon = NULL;
	pCurrentWeapon = NULL;
}

bool MHero::GetNearLevelExitPrev() {
	return NearLevelExitPrev;
}

bool MHero::GetNearLevelExitNext() {
	return NearLevelExitNext;
}

void MHero::OnBeginCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type) {
	vector<MEnemy*>::iterator itv;
	switch(Type) {
		case OT_LEPREV:
			NearLevelExitPrev = true;
			break;
		case OT_LENEXT:
			NearLevelExitNext = true;
			break;
		case OT_ENBODY:
			itv = find(ContactEnemies.begin(), ContactEnemies.end(), (MEnemy*)pPhysicQuad);
			if(itv == ContactEnemies.end()) {
				ContactEnemies.push_back((MEnemy*)pPhysicQuad);
			}	
			break;
	};
}

void MHero::OnEndCollideWithF(MPhysicQuad* pPhysicQuad, uint16 Type) {
	vector<MEnemy*>::iterator itv;
	switch(Type) {
		case OT_LEPREV:
			NearLevelExitPrev = false;
			break;
		case OT_LENEXT:
			NearLevelExitNext = false;
			break;
		case OT_ENBODY:
			itv = find(ContactEnemies.begin(), ContactEnemies.end(), (MEnemy*)pPhysicQuad);
			if(itv != ContactEnemies.end()) {
				ContactEnemies.erase(itv);
			}
			break;
	};
}

void MHero::OnBeginWallCollide(){}
void MHero::OnEndWallCollide(){}

void MHero::ProcessContactEnemies() {
	vector<MEnemy*>::iterator itv;
	itv = ContactEnemies.begin();
	while(itv != ContactEnemies.end()) {
		cout<<"Damage from: "<<*itv<<endl;
		itv ++;
	}
}

void MHero::EquipWeapon(MWeapon* pWeapon) {
	if(!pWeapon) {
		return;
	}
	if(pCurrentWeapon) {
		if(pWeapon == pCurrentWeapon) {
			return;
		}
		else {
			pWorld->DestroyJoint(WeaponJoint);
		}
	}
	
	b2RevoluteJointDef JointDef;
	float Anchor = (GetSize().x - pCurrentWeapon->GetSize().x) / 2 * PhysicScale;
	pCurrentWeapon = pWeapon;
	JointDef.bodyA = Body;
	JointDef.bodyB = pCurrentWeapon->GetBody();
	JointDef.collideConnected = false;
	JointDef.localAnchorA.Set(0, 0);
	JointDef.localAnchorB.Set(Anchor, 0);
	WeaponJoint = (b2RevoluteJoint*)World->CreateJoint(&JointDef);
	pCurrentWeapon->GetBody()->SetTransform(Body->GetPosition() - b2Vec2(Anchor, 0), 0);
	pCurrentWeapon->GetBody()->SetActive(true);
	//add to buffer
}

void MHero::UnequipWeapon() {
	if(!pCurrentWeapon) {
		return;
	}
	pWorld->DestroyJoint(WeaponJoint);
	pCurrentWeapon = NULL;
	//remove from buffer
}

void MHero::Attack(glm::vec2 MousePoint) {
	if(!pCurrentWeapon) return;
	WeaponJoint->EnableMotor(false);
	float Anchor = -(GetSize().x - pCurrentWeapon->GetSize().x) / 2 * PhysicScale;
	b2Vec2 HeroCenter = Body->GetPosition();
	glm::vec2 Direction = glm::normalize(MousePoint - glm::vec2(HeroCenter.x / PhysicScale, HeroCenter.y / PhysicScale));
	Angle = atan2(Direction.y, Direction.x);
	Direction = glm::rotate(Direction, pCurrentWeapon->GetSwingAnle());
	Direction = glm::normalize(Direction);
	HeroCenter.x += Direction.x * Anchor;
	HeroCenter.y += Direction.y * Anchor;
	pCurrentWeapon->GetBody()->SetTransform(HeroCenter, Angle + pCurrentWeapon->GetSwingAnle());
	WeaponJoint->SetLimits(Angle - pCurrentWeapon->GetSwingAnle(), Angle + pCurrentWeapon->GetSwingAnle());
	WeaponJoint->EnableLimit(true);
	WeaponJoint->SetMotorSpeed(-pCurrentWeapon->GetAttackSpeed());
  	WeaponJoint->SetMaxMotorTorque(5);
	WeaponJoint->EnableMotor(true);
	pCurrentWeapon->BeginAttack(Angle);
}

MWeapon* MHero::GetCurrentWeapon() {
	return pCurrentWeapon;
}

void MHero::Update() {
	MPhysicQuad::Update();
	MLiving::LevelUp();
}

