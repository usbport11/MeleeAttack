#include "../../stdafx.h"
#include "Living.h"

MLiving::MLiving():MPhysicQuad() {
	Health = 0;
	Expirience = 0;
	Level = 0;
	NextLevelExpirience = glm::exp(Level) * LEVEL_EXP_VALUE;
}

void MLiving::ReciveDamage(int Value) {
	Health -= Value;
}

void MLiving::SetHealth(int Value) {
	Health = Value;
}

int MLiving::GetHealth() {
	return Health;
}

void MLiving::SetExpirience(int Value) {
	Expirience = Value;
}

int MLiving::GetExpirience() {
	return Expirience;
}

void MLiving::AddExpirience(int Value) {
	Expirience += Value;
}

void MLiving::LevelUp() {
	if(Expirience > NextLevelExpirience) {
		Level ++;
		Expirience -= NextLevelExpirience;
		NextLevelExpirience = glm::exp(Level) * LEVEL_EXP_VALUE;
		cout<<"Level up! To next: "<<NextLevelExpirience<<endl;
	}
}

