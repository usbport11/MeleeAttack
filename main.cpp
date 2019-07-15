#include "stdafx.h"
#include "classes/system/Shader.h"
#include "classes/system/Scene.h"
#include "classes/system/FPSController.h"
#include "classes/buffers/QuadBuffer.h"
#include "classes/objects/PhysicQuad.h"
#include "classes/objects/ContactListener.h"

#define WNR_NONE 0
#define WNR_UNEQUIP 1
#define WNR_EQUIP 2

bool Pause;
bool keys[1024] = {0};
int WindowWidth = 800, WindowHeight = 600;
bool EnableVsync = 1;
GLFWwindow* window;
stFPSController FPSController;
tVoidFunc DrawFunc = NULL;

glm::vec2 MouseSceneCoord;
float Angle;
//atack data
bool AttackInProgress;
int WeaponChangeRequest;
float PreAngle = 45 * PH_DEGTORAD;
float AttackSpeed = 10.0;
glm::vec2 WeaponAnchor = glm::vec2(-0.25, 0); //at 0 degrees []---
float AnchorLength = glm::length(WeaponAnchor);
glm::vec2 Weapon1Size = glm::vec2(100, 20);
glm::vec2 Weapon2Size = glm::vec2(120, 30);
glm::vec2 HeroSize = glm::vec2(50, 50);

MShader Shader;
MScene Scene;

bool MeleeAttack = false;
MPhysicQuad* Weapon1 = NULL;
MPhysicQuad* Weapon2 = NULL;
MPhysicQuad* pCurrentWeapon = NULL;
MPhysicQuad* pWeaponToChange = NULL;
MPhysicQuad* Hero;
MPhysicQuad* Enemy;
MPhysicQuad* Wall;

MQuadBuffer QuadBuffer;
glm::vec2 QuadVelocity = glm::vec2(1.0, 1.0);
glm::vec2 QuadSize = glm::vec2(10, 10);
glm::vec3 QuadColor = glm::vec3(0, 1, 0);
b2RevoluteJointDef JointDef;
b2RevoluteJoint* Joint;

b2World* World;
b2Vec2 Gravity;
float timeStep;
int velocityIterations;
int positionIterations;
float PhysicScale;//100 pixel - 1 meter
MContactListener CL;

void DrawDungeon();
void UnequipWeapon();
void EquipWeapon(MPhysicQuad* pWeapon, glm::vec2 Anchor);

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void mousepos_callback(GLFWwindow* window, double x, double y) {
	MouseSceneCoord = Scene.WindowPosToWorldPos(x, y);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if(action == GLFW_PRESS && pCurrentWeapon) {
		cout<<"Attack start"<<endl;
		AttackInProgress = true;
		Joint->EnableMotor(false);
		b2Vec2 HeroCenter = Hero->GetBody()->GetPosition();
		glm::vec2 Direction = glm::normalize(MouseSceneCoord - glm::vec2(HeroCenter.x / PhysicScale, HeroCenter.y / PhysicScale));
		Angle = atan2(Direction.y, Direction.x);
		Direction = glm::rotate(Direction, PreAngle);//new
		Direction = glm::normalize(Direction);//new
		HeroCenter.x += Direction.x * AnchorLength;
		HeroCenter.y += Direction.y * AnchorLength;
		pCurrentWeapon->GetBody()->SetTransform(HeroCenter, Angle + PreAngle);//+ 0
		Joint->SetLimits(Angle - PreAngle, Angle + PreAngle);
		Joint->EnableLimit(true);
		Joint->SetMotorSpeed(-AttackSpeed);
  		Joint->SetMaxMotorTorque(5);
		Joint->EnableMotor(true);
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}
	
	if(!pWeaponToChange){
		if(key == '1' && action == GLFW_PRESS) {
			cout<<"Try equip first weapon"<<endl;
			pWeaponToChange = Weapon1;
			WeaponChangeRequest = WNR_EQUIP;
			WeaponAnchor.x = (HeroSize.x - Weapon1Size.x) / 2 * PhysicScale;
			AnchorLength = fabs(WeaponAnchor.x);
			return;
		}
		if(key == '2' && action == GLFW_PRESS) {
			cout<<"Try equip second weapon"<<endl;
			pWeaponToChange = Weapon2;
			WeaponChangeRequest = WNR_EQUIP;
			WeaponAnchor.x = (HeroSize.x - Weapon2Size.x) / 2 * PhysicScale;
			AnchorLength = fabs(WeaponAnchor.x);
			return;
		}
	}
	if(key == '3' && action == GLFW_PRESS) {
		cout<<"Try unequip weapon"<<endl;
		pWeaponToChange = NULL;
		WeaponChangeRequest = WNR_UNEQUIP;
		return;	
	}
	
	if(action == GLFW_PRESS)
    	keys[key] = true;
    else if (action == GLFW_RELEASE)
    	keys[key] = false;
}

void KeyActions() {
	b2Vec2 CurrentVelocity = b2Vec2(0.0f, 0.0f);
	if(keys[GLFW_KEY_LEFT] || keys[GLFW_KEY_RIGHT] || keys[GLFW_KEY_UP] || keys[GLFW_KEY_DOWN]) {
		if(keys[GLFW_KEY_LEFT]) {
			CurrentVelocity.x = -QuadVelocity.x;
		}
		if(keys[GLFW_KEY_RIGHT]) {
			CurrentVelocity.x = QuadVelocity.x;
		}
		if(keys[GLFW_KEY_UP]) {
			CurrentVelocity.y = QuadVelocity.y;
		}
		if(keys[GLFW_KEY_DOWN]) {
			CurrentVelocity.y = -QuadVelocity.y;
		}
	}
	Hero->SetVelocity(CurrentVelocity);
}

bool CheckOpenglSupport() {
	//get opengl data (here was CRASH on suspisious notebook)
	string OpenGLVersion = (char*)glGetString(GL_VERSION);
	string OpenGLVendor = (char*)glGetString(GL_VENDOR);
	string OpenGLRenderer = (char*)glGetString(GL_RENDERER);
	string ShadersVersion = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
	LogFile<<"Window: OpenGL version: "<<OpenGLVersion<<endl;
	LogFile<<"Window: OpenGL vendor: "<<OpenGLVendor<<endl;
	LogFile<<"Window: OpenGL renderer: "<<OpenGLRenderer<<endl;
	LogFile<<"Window: Shaders version: "<<ShadersVersion<<endl;
	
	float VersionOGL, VersionSHD;
	sscanf(OpenGLVersion.c_str(), "%f", &VersionOGL);
	if(VersionOGL < 3.0f) {
		LogFile<<"Window: Old version of OpenGL. Sorry"<<endl;
		return false;
	}
	sscanf(ShadersVersion.c_str(), "%f", &VersionSHD);
	if(VersionSHD < 3.3f) {
		LogFile<<"Window: Old version of shaders. Sorry"<<endl;
		return false;
	}
}

bool InitApp() {
	LogFile<<"Starting application"<<endl;    
    glfwSetErrorCallback(error_callback);
    
    if(!glfwInit()) return false;
    window = glfwCreateWindow(WindowWidth, WindowHeight, "TestApp", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return false;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mousepos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwMakeContextCurrent(window);
    if(glfwExtensionSupported("WGL_EXT_swap_control")) {
    	LogFile<<"Window: V-Sync supported. V-Sync: "<<EnableVsync<<endl;
		glfwSwapInterval(EnableVsync);//0 - disable, 1 - enable
	}
	else LogFile<<"Window: V-Sync not supported"<<endl;
    LogFile<<"Window created: width: "<<WindowWidth<<" height: "<<WindowHeight<<endl;

	//glew
	GLenum Error = glewInit();
	if(GLEW_OK != Error) {
		LogFile<<"Window: GLEW Loader error: "<<glewGetErrorString(Error)<<endl;
		return false;
	}
	LogFile<<"GLEW initialized"<<endl;
	
	if(!CheckOpenglSupport()) return false;

	//shaders
	if(!Shader.CreateShaderProgram("shaders/main.vertexshader.glsl", "shaders/main.fragmentshader.glsl")) return false;
	if(!Shader.PrepareShaderValues()) return false;
	LogFile<<"Shaders loaded"<<endl;

	//scene
	if(!Scene.Initialize(&WindowWidth, &WindowHeight)) return false;
	LogFile<<"Scene initialized"<<endl;

	//randomize
    srand(time(NULL));
    LogFile<<"Randomized"<<endl;
    
    //world
    Gravity = b2Vec2(0, 0);
    World = new b2World(Gravity);
    World->SetContactListener(&CL);
	timeStep = 1 / 60.0;
	velocityIterations = 8;
	positionIterations = 3;
	PhysicScale = 0.01;
    
	//create objects
	Weapon1 = new MPhysicQuad;
	Weapon1->Create(glm::vec2(100,100), Weapon1Size, glm::vec3(0,1,0), PhysicScale, World, b2_dynamicBody, OT_WEAPON, OT_ENBODY, true);
	Weapon1->GetBody()->SetActive(false);
	Weapon2 = new MPhysicQuad;
	Weapon2->Create(glm::vec2(100,100), Weapon2Size, glm::vec3(0,1,0), PhysicScale, World, b2_dynamicBody, OT_WEAPON, OT_ENBODY, true);
	Weapon2->GetBody()->SetActive(false);
	Hero = new MPhysicQuad;
	Hero->Create(glm::vec2(40,100), HeroSize, glm::vec3(1,1,1), PhysicScale, World, b2_dynamicBody, OT_HERO, OT_WALL, false);
	Enemy = new MPhysicQuad;
	Enemy->Create(glm::vec2(200,200), HeroSize, glm::vec3(1,0,0), PhysicScale, World, b2_dynamicBody, OT_ENBODY, OT_WEAPON, false);
	Wall = new MPhysicQuad;
	Wall->Create(glm::vec2(300,100), glm::vec2(50,100), glm::vec3(0,0,1), PhysicScale, World, b2_staticBody, OT_WALL, OT_ALL, false);
	Hero->GetBody()->SetFixedRotation(true);
	Weapon1->SetNullMass();
	Weapon2->SetNullMass();
	
	//add to buffer
	QuadBuffer = MQuadBuffer(GL_STREAM_DRAW);
	QuadBuffer.AddQuad(Hero->GetColorQuad());
	QuadBuffer.AddQuad(Enemy->GetColorQuad());
	QuadBuffer.AddQuad(Wall->GetColorQuad());
	QuadBuffer.Relocate();
	
	//draw func
	DrawFunc = &DrawDungeon;
	Pause = false;
    
    return true;
}

void ProcessWeapon() {
	switch(WeaponChangeRequest) {
		case WNR_UNEQUIP:
			UnequipWeapon();
			WeaponChangeRequest = WNR_NONE;
			pWeaponToChange = NULL;
			break;
		case WNR_EQUIP:
			EquipWeapon(pWeaponToChange, WeaponAnchor);
			WeaponChangeRequest = WNR_NONE;
			pWeaponToChange = NULL;
			break;
	}
	if(AttackInProgress && pCurrentWeapon->GetBody()->GetAngle() >= Angle - PreAngle) {
		AttackInProgress = false;
		cout<<"Attack stop"<<endl;
	}
}

void UnequipWeapon() {
	if(!pCurrentWeapon) {
		cout<<"No current weapon"<<endl;
		return;
	}
	if(!QuadBuffer.IsReady()) {
		cout<<"Buffer not ready"<<endl;
		return;
	}
	World->DestroyJoint(Joint);
	QuadBuffer.RemoveQuad(pCurrentWeapon->GetColorQuad());
	QuadBuffer.Relocate();
	pCurrentWeapon = NULL;
}

void EquipWeapon(MPhysicQuad* pWeapon, glm::vec2 Anchor) {
	if(!pWeapon) {
		cout<<"No weapon"<<endl;
		return;
	}
	if(!QuadBuffer.IsReady()) {
		cout<<"Buffer not ready"<<endl;
		return;
	}
	if(pCurrentWeapon) {
		if(pWeapon == pCurrentWeapon) {
			cout<<"Same weapon"<<endl;
			return;
		}
		else {
			World->DestroyJoint(Joint);
			QuadBuffer.RemoveQuad(pCurrentWeapon->GetColorQuad());
		}
	}
	
	pCurrentWeapon = pWeapon;
	JointDef.bodyA = Hero->GetBody();
	JointDef.bodyB = pCurrentWeapon->GetBody();
	JointDef.collideConnected = false;
	JointDef.localAnchorA.Set(0, 0);
	JointDef.localAnchorB.Set(Anchor.x, Anchor.y);
	Joint = (b2RevoluteJoint*)World->CreateJoint(&JointDef);
	pCurrentWeapon->GetBody()->SetTransform(Hero->GetBody()->GetPosition() - b2Vec2(Anchor.x, 0), 0);
	pCurrentWeapon->GetBody()->SetActive(true);
	QuadBuffer.AddQuad(pCurrentWeapon->GetColorQuad());
	QuadBuffer.Relocate();
	cout<<"Anc: "<<Anchor.x<<" "<<pCurrentWeapon->GetBody()->GetPosition().x<<endl;
}

void DrawDungeon() {
	if(!Pause) {
		ProcessWeapon();
		KeyActions();
		World->Step(timeStep, velocityIterations, positionIterations);
		if(pCurrentWeapon) pCurrentWeapon->Update();
		Hero->Update();
		QuadBuffer.UpdateAll();
	}
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(Shader.ProgramId);
	glUniformMatrix4fv(Shader.MVPId, 1, GL_FALSE, Scene.GetDynamicMVP());
	
	glEnableVertexAttribArray(SHR_LAYOUT_VERTEX);
	glEnableVertexAttribArray(SHR_LAYOUT_COLOR);
	
	QuadBuffer.DrawAll();
	
	glDisableVertexAttribArray(SHR_LAYOUT_VERTEX);
	glDisableVertexAttribArray(SHR_LAYOUT_COLOR);
}

void DrawLoad() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderStep() {
	(*DrawFunc)();
}

void ClearApp() {
	LogFile<<"Closing application. Free memory"<<endl;
	if(Hero) {
		Hero->Close();
		delete Hero;
	}
	if(Weapon1) {
		Weapon1->Close();
		delete Weapon1;
	}
	if(Weapon2) {
		Weapon2->Close();
		delete Weapon2;
	}
	if(Enemy) {
		Enemy->Close();
		delete Enemy;
	}
	if(Wall) {
		Wall->Close();
		delete Wall;
	}
	if(World) delete World;
	QuadBuffer.Close();
	Shader.Close();
}

int main(int argc, char** argv) {
	if(!InitApp()) {
		ClearApp();
		glfwTerminate();
		return 0;
	}
	FPSController.Initialize(glfwGetTime());
	while(!glfwWindowShouldClose(window)) {
		FPSController.FrameStep(glfwGetTime());
    	FPSController.FrameCheck();
		RenderStep();
        glfwSwapBuffers(window);
        glfwPollEvents();
	}
	ClearApp();
    glfwTerminate();
	
	return 0;
}
