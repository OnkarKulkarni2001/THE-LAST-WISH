#include "cLevelGenerator.h"
#include <vector>
#include "sMesh.h"
#include "PhysXWraper/cPhysXWraper.h"
#include "PhysX/PxPhysicsAPI.h"
#include "Animation/Animator.h"
#include "sharedThings.h"
#include "cMasterZombie.h"
#include "cKey.h"
#include "Health.h"
#include "cAmmo.h"
#include "cPhysics.h"
#include "sharedThings.h"




using namespace physx;

std::vector<sMesh*> vecCeilings;

extern std::vector<sMesh*> g_vecMeshesToDraw;
extern std::vector<sModelAnimatorLink*> vecModelAnimatorLinks;
extern std::vector<cMasterZombie*> AllZombies;
extern cLevelGenerator* pLevelGenerator;
extern std::vector<cKey*> g_pVecInteractableKeys;
extern std::vector<cHealth*> g_pVecInteractableHealth;
extern std::vector<cAmmo*> g_pVecInteractableAmmo;
extern cPhysics* g_pPhysicEngine;
extern Animator* g_pZombieAnimator;

glm::vec3 lightMeshOffset = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 healthOffset = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 ammoOffset = glm::vec3(0.0f, 0.0f, 1.0f);

glm::vec4 lightOffset = glm::vec4(0.0f, 0.0f, 1.5f, 0.0f);


std::vector<sMesh*> g_vecDoors;
sMesh* g_pFindMeshByFriendlyName(std::string theNameToFind);

void createStaticWall(const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation);
//void createStaticWall(PxRigidStatic*& rigidDoor, const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation);
void createDynamicWall(const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation);

cLevelGenerator::cLevelGenerator()
{
	this->xRows = 0;
	this->yColoums = 0;
}

cLevelGenerator::~cLevelGenerator()
{

}

void cLevelGenerator::SetupMap(int x, int y)
{
	this->xRows = x - 1;
	this->yColoums = y - 1;
	mapData = new char* [xRows];
	for (int i = 0; i < xRows; i++)
	{
		mapData[i] = new char[yColoums];
	}
}

bool isInsideMap(int i, int j, int rows, int cols) {
	return i > 0 && i < rows - 1 && j > 0 && j < cols - 1;
}

bool isSurroundedByFloor(int i, int j, char** mapData) {
	return mapData[i - 1][j] == 'X' &&
		mapData[i + 1][j] == 'X' &&
		mapData[i][j - 1] == 'X' &&
		mapData[i][j + 1] == 'X';
}

void cLevelGenerator::ReadMap(std::string path)
{
	// Initialize mapData to a default value (e.g., space)
	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			mapData[i][j] = ' ';
		}
	}

	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open map file: " << path << std::endl;
		return;
	}

	std::string line;
	int row = 0;

	while (std::getline(file, line) && row < xRows)
	{
		for (int col = 0; col < yColoums && col < (int)line.length(); ++col)
		{
			mapData[row][col] = line[col];
		}
		row++;
	}
	std::cerr << "Successfully Read map file: " << path << std::endl;
	file.close();
}

//void tryPlaceKey(char keySymbol, int i, int j, int scale, char** mapData) {
//	if (!isInsideMap(i, j, pLevelGenerator->xRows, pLevelGenerator->yColoums)) return;
//	if (!isSurroundedByFloor(i, j, mapData)) return;
//
//	pLevelGenerator->AddFloor(i * scale, j * scale);
//
//	switch (keySymbol) {
//	case 'R': pLevelGenerator->AddRedKey(i * scale, j * scale); break;
//	case 'Y': pLevelGenerator->AddYellowKey(i * scale, j * scale); break;
//	case 'G': pLevelGenerator->AddGreenKey(i * scale, j * scale); break;
//	case 'B': pLevelGenerator->AddBlueKey(i * scale, j * scale); break;
//	}
//}
void tryPlaceKey(char keySymbol, int i, int j, int scale, char** mapData)
{
	if (!isInsideMap(i, j, pLevelGenerator->xRows, pLevelGenerator->yColoums)) return;
	if (!isSurroundedByFloor(i, j, mapData)) return;

	// Place floor under the key
	pLevelGenerator->AddFloor(i * scale, j * scale);

	// Create new key
	cKey* pKey = new cKey();

	// Determine mesh name based on key symbol
	std::string meshName;
	switch (keySymbol) {
	case 'R': meshName = "RedKey"; break;
	case 'Y': meshName = "YellowKey"; break;
	case 'G': meshName = "GreenKey"; break;
	case 'B': meshName = "BlueKey"; break;
	default: return; // Invalid key symbol, safety check
	}

	// Find the mesh
	sMesh* pKeyMesh = g_pFindMeshByFriendlyName(meshName);

	if (!pKeyMesh) {
		std::cerr << "Error: Mesh not found for key symbol '" << keySymbol << "'\n";
		delete pKey;
		return;
	}

	// Initialize the key
	pKey->InteractableBeginPlay(pKeyMesh);

	// Set key position and scale
	pKey->pInteractableTrigger->positionXYZ = glm::vec3(i * scale, 0.0f, j * scale);
	pKey->pInteractableTrigger->uniformScale = 2.0f;

	// Add to interactable keys vector
	g_pVecInteractableKeys.push_back(pKey);
}



float scale = 2.0f;

void cLevelGenerator::GenerateMap()
{
	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			if (mapData[i][j] == '0')
			{
				//Add Nothing
			}
			else if (mapData[i][j] == 'D') {
				AddFloor(i * scale, j * scale);
				if (j == 0 || mapData[i][j - 1] == '0')
				{
					//std::cout << "Add wall to the LEFT of (" << i << ", " << j << ")" << std::endl;
					AddDoor((i * scale), (j * scale), 90.0f);
					AddWall((i * scale), (j * scale), 0.0f);
				}

				// RIGHT
				if (j == yColoums - 1 || mapData[i][j + 1] == '0')
				{
					//std::cout << "Add wall to the RIGHT of (" << i << ", " << j << ")" << std::endl;
					AddDoor((i * scale), (j * scale) + scale, -90.0f);
					AddWall((i * scale), (j * scale) + scale, 0.0f);
				}

				// TOP
				if (i == 0 || mapData[i - 1][j] == '0')
				{
					//std::cout << "Add wall to the TOP of (" << i << ", " << j << ")" << std::endl;
					AddDoor((i * scale) - scale, (j * scale), 180.0f);
					AddWall((i * scale) - scale, (j * scale), 90.0f);
				}

				// BOTTOM
				if (i == xRows - 1 || mapData[i + 1][j] == '0')
				{
					//std::cout << "Add wall to the BOTTOM of (" << i << ", " << j << ")" << std::endl;
					AddDoor((i * scale), (j * scale), 0.0f);
					AddWall((i * scale), (j * scale), 90.0f);
				}
			}
			//else if (mapData[i][j] == 'R' || mapData[i][j] == 'Y' ||
			//	mapData[i][j] == 'G' || mapData[i][j] == 'B') {
			//	tryPlaceKey(mapData[i][j], i, j, scale, mapData);
			//}

			else if (mapData[i][j] == 'X')
			{

				AddFloor(i * scale, j * scale);
				AddCeiling(i * scale, j * scale);

				// LEFT
				if (j == 0 || mapData[i][j - 1] == '0')
				{
					//std::cout << "Add wall to the LEFT of (" << i << ", " << j << ")" << std::endl;
					AddWall((i * scale), (j * scale), 0.0f);
				}

				// RIGHT
				if (j == yColoums - 1 || mapData[i][j + 1] == '0')
				{
					//std::cout << "Add wall to the RIGHT of (" << i << ", " << j << ")" << std::endl;
					AddWall((i * scale), (j * scale) + scale, 0.0f);
				}

				// TOP
				if (i == 0 || mapData[i - 1][j] == '0')
				{
					//std::cout << "Add wall to the TOP of (" << i << ", " << j << ")" << std::endl;
					AddWall((i * scale) - scale, (j * scale), 90.0f);
				}

				// BOTTOM
				if (i == xRows - 1 || mapData[i + 1][j] == '0')
				{
					//std::cout << "Add wall to the BOTTOM of (" << i << ", " << j << ")" << std::endl;
					AddWall((i * scale), (j * scale), 90.0f);
				}
			}
		}
	}

	//GenerateLights("assets/Misc/LightMap_Output.txt");
	//GenerateZombies("assets/Misc/ZombieMap.txt");
	GenerateZombies("assets/Misc/ZombieMap - Copy.txt");
	GenerateKeysOnMap("assets/Misc/KeyMap.txt");
	GenerateHealthAndAmmo("assets/Misc/HealthAmmo.txt");
}

int numCeilingLight = 0;
int numLight = 2;

void cLevelGenerator::GenerateHealthAndAmmo(std::string path)
{
	// Initialize mapData to a default value (e.g., space)
	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			mapData[i][j] = ' ';
		}
	}

	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open map file: " << path << std::endl;
		return;
	}

	std::string line;
	int row = 0;

	while (std::getline(file, line) && row < xRows)
	{
		for (int col = 0; col < yColoums && col < (int)line.length(); ++col)
		{
			mapData[row][col] = line[col];
		}
		row++;
	}
	std::cerr << "Successfully Read map file: " << path << std::endl;
	file.close();

	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			if (mapData[i][j] == 'A') 
			{
				AddAmmo(i * scale, j * scale);
			}
			else if (mapData[i][j] == 'H') 
			{
				AddHealth(i * scale, j * scale);
			}
		}
	}
}

void cLevelGenerator::AddLight(float x, float y, float roty)
{

	sMesh* pCeilingLight = new sMesh();
	pCeilingLight->modelFileName = "assets/LevelModels/CeilingLight.ply";
	pCeilingLight->positionXYZ = glm::vec3(x, 4.0f, y) + lightMeshOffset;
	pCeilingLight->rotationEulerXYZ = glm::vec3(0.0f, roty, 0.0f);
	pCeilingLight->uniformScale = 1.0f;
	pCeilingLight->bDoNotLight = false;
	pCeilingLight->bIsWireframe = false;
	pCeilingLight->uniqueFriendlyName = "CeilingLight" + std::to_string(numCeilingLight);
	pCeilingLight->textures[0] = "SM_overhead_light_T_overhead_light_2_BaseColor.bmp";
	pCeilingLight->textures[5] = "SM_overhead_light_T_overhead_light_2_BaseColor_NRM.bmp";
	pCeilingLight->hasNormalMap = true;
	pCeilingLight->blendRatio[0] = 1.0f;

	numCeilingLight++;
	::g_vecMeshesToDraw.push_back(pCeilingLight);

	//Set up one of the lights in the scene
	::g_pLightManager->theLights[numLight].position = glm::vec4(x, 3.5, y, 1.0f) + lightOffset;
	::g_pLightManager->theLights[numLight].diffuse = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].atten.y = 1.0f;
	::g_pLightManager->theLights[numLight].atten.z = 0.01f;

	::g_pLightManager->theLights[numLight].param1.x = 0.0f;    // Point light (see shader)
	::g_pLightManager->theLights[numLight].param2.x = 1.0f;    // Turn on (see shader)

	numLight++;

}

void cLevelGenerator::GenerateLights(std::string path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open light map file: " << path << std::endl;
		return;
	}

	std::string line;
	int row = 0;

	while (std::getline(file, line))
	{
		for (int col = 0; col < line.size(); ++col)
		{
			if (line[col] == 'L')
			{
				float x = static_cast<float>(col);
				float y = static_cast<float>(row);
				float roty = 0.0f; // Default rotation, you can customize this if needed

				AddLight(x * scale, y  * scale, roty);
			}
		}
		row++;
	}

	file.close();
}

void cLevelGenerator::GenerateZombies(std::string path)
{
	// Initialize mapData to a default value (e.g., space)
	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			mapData[i][j] = ' ';
		}
	}

	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open map file: " << path << std::endl;
		return;
	}

	std::string line;
	int row = 0;

	while (std::getline(file, line) && row < xRows)
	{
		for (int col = 0; col < yColoums && col < (int)line.length(); ++col)
		{
			mapData[row][col] = line[col];
		}
		row++;
	}
	std::cerr << "Successfully Read map file: " << path << std::endl;
	file.close();

	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			if (mapData[i][j] == 'Z') {
				AddZombie(i * scale, j * scale);
			}
		}
	}
}

void cLevelGenerator::GenerateKeysOnMap(std::string path)
{
	// Initialize mapData to a default value (e.g., space)
	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			mapData[i][j] = ' ';
		}
	}

	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open map file: " << path << std::endl;
		return;
	}

	std::string line;
	int row = 0;

	while (std::getline(file, line) && row < xRows)
	{
		for (int col = 0; col < yColoums && col < (int)line.length(); ++col)
		{
			mapData[row][col] = line[col];
		}
		row++;
	}
	std::cerr << "Successfully Read map file: " << path << std::endl;
	file.close();

	for (int i = 0; i < xRows; ++i)
	{
		for (int j = 0; j < yColoums; ++j)
		{
			if (mapData[i][j] == 'R') {
				AddFloor(i * scale, j * scale);
				AddRedKey(i * scale, j * scale);
			}
			else if (mapData[i][j] == 'Y') {
				AddFloor(i * scale, j * scale);
				AddYellowKey(i * scale, j * scale);
			}
			else if (mapData[i][j] == 'G') {
				AddFloor(i * scale, j * scale);
				AddGreenKey(i * scale, j * scale);
			}
			else if (mapData[i][j] == 'B') {
				AddFloor(i * scale, j * scale);
				AddBlueKey(i * scale, j * scale);
			}
		}
	}
}

int numFloor = 0;
int numDoor = 0;
void cLevelGenerator::AddFloor(float x, float y)
{
	sMesh* pFloor = new sMesh();
	pFloor->modelFileName = "assets/LevelModels/Floor.ply";
	pFloor->positionXYZ = glm::vec3(x, 1.0f, y);
	pFloor->rotationEulerXYZ = glm::vec3(0.0f);
	pFloor->uniformScale = 1.0f;
	pFloor->bDoNotLight = false;
	pFloor->bIsWireframe = false;
	pFloor->uniqueFriendlyName = "Floor" + std::to_string(numFloor);
	pFloor->textures[0] = "Floor.bmp";
	pFloor->textures[5] = "Floor_NRM.bmp";
	pFloor->hasNormalMap = true;
	pFloor->blendRatio[0] = 1.0f;

	numFloor++;
	::g_vecMeshesToDraw.push_back(pFloor);

	g_pPhysicEngine->addTriangleMesh("assets/LevelModels/Floor.ply",
		pFloor->positionXYZ,
		pFloor->rotationEulerXYZ,
		pFloor->uniformScale);
}

void cLevelGenerator::AddRedKey(float x, float y)
{
	sMesh* pRedKey = new sMesh();
	pRedKey->modelFileName = "assets/LevelModels/RedKey.ply";
	pRedKey->positionXYZ = glm::vec3(x, 2.0f, y);
	pRedKey->rotationEulerXYZ = glm::vec3(-90.0f, 0.0f, 0.0f);
	pRedKey->uniformScale = 3.0f;
	pRedKey->bDoNotLight = false;
	pRedKey->bIsWireframe = false;
	pRedKey->uniqueFriendlyName = "RedKey";
	pRedKey->textures[0] = "Keys.bmp";
	pRedKey->textures[5] = "Keys_NRM.bmp";
	pRedKey->hasNormalMap = true;
	pRedKey->blendRatio[0] = 1.0f;

	::g_vecMeshesToDraw.push_back(pRedKey);

	//// Set up one of the lights in the scene
	::g_pLightManager->theLights[numLight].position = glm::vec4(x, 4.0f, y, 1.0f);
	::g_pLightManager->theLights[numLight].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].atten.y = 0.1f;
	::g_pLightManager->theLights[numLight].atten.z = 0.1f;

	::g_pLightManager->theLights[numLight].param1.x = 1.0f;    // Spot light (see shader)
	::g_pLightManager->theLights[numLight].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].param1.y = 0.0f;   //  y = inner angle
	::g_pLightManager->theLights[numLight].param1.z = 25.0f;  //  z = outer angle

	::g_pLightManager->theLights[numLight].param2.x = 1.0f;    // Turn on (see shader)

	numLight++;
}

void cLevelGenerator::AddGreenKey(float x, float y)
{
	sMesh* pGreenKey = new sMesh();
	pGreenKey->modelFileName = "assets/LevelModels/GreenKey.ply";
	pGreenKey->positionXYZ = glm::vec3(x, 2.0f, y);
	pGreenKey->rotationEulerXYZ = glm::vec3(-90.0f, 0.0f, 0.0f);
	pGreenKey->uniformScale = 3.0f;
	pGreenKey->bDoNotLight = false;
	pGreenKey->bIsWireframe = false;
	pGreenKey->uniqueFriendlyName = "GreenKey";
	pGreenKey->textures[0] = "Keys.bmp";
	pGreenKey->textures[5] = "Keys_NRM.bmp";
	pGreenKey->hasNormalMap = true;
	pGreenKey->blendRatio[0] = 1.0f;

	::g_vecMeshesToDraw.push_back(pGreenKey);

	//// Set up one of the lights in the scene
	::g_pLightManager->theLights[numLight].position = glm::vec4(x, 4.0f, y, 1.0f);
	::g_pLightManager->theLights[numLight].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].atten.y = 0.1f;
	::g_pLightManager->theLights[numLight].atten.z = 0.1f;

	::g_pLightManager->theLights[numLight].param1.x = 1.0f;    // Spot light (see shader)
	::g_pLightManager->theLights[numLight].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].param1.y = 0.0f;   //  y = inner angle
	::g_pLightManager->theLights[numLight].param1.z = 25.0f;  //  z = outer angle

	::g_pLightManager->theLights[numLight].param2.x = 1.0f;    // Turn on (see shader)

	numLight++;
}

void cLevelGenerator::RotateKeys(float deltaTime) {
	sMesh* redKey = g_pFindMeshByFriendlyName("RedKey");
	if(redKey) redKey->rotationEulerXYZ.z += 120.0f * deltaTime;
	sMesh* yellowKey = g_pFindMeshByFriendlyName("YellowKey");
	if(yellowKey) yellowKey->rotationEulerXYZ.z += 120.0f * deltaTime;
	sMesh* greenKey = g_pFindMeshByFriendlyName("GreenKey");
	if(greenKey) greenKey->rotationEulerXYZ.z += 120.0f * deltaTime;
	sMesh* blueKey = g_pFindMeshByFriendlyName("BlueKey");
	if(blueKey) blueKey->rotationEulerXYZ.z += 120.0f * deltaTime;
}

void cLevelGenerator::AddYellowKey(float x, float y)
{
	sMesh* pYellowKey = new sMesh();
	pYellowKey->modelFileName = "assets/LevelModels/YellowKey.ply";
	pYellowKey->positionXYZ = glm::vec3(x, 2.0f, y);
	pYellowKey->rotationEulerXYZ = glm::vec3(-90.0f, 0.0f, 0.0f);
	pYellowKey->uniformScale = 3.0f;
	pYellowKey->bDoNotLight = false;
	pYellowKey->bIsWireframe = false;
	pYellowKey->uniqueFriendlyName = "YellowKey";
	pYellowKey->textures[0] = "Keys.bmp";
	pYellowKey->textures[5] = "Keys_NRM.bmp";
	pYellowKey->hasNormalMap = true;
	pYellowKey->blendRatio[0] = 1.0f;

	::g_vecMeshesToDraw.push_back(pYellowKey);

	//// Set up one of the lights in the scene
	::g_pLightManager->theLights[numLight].position = glm::vec4(x, 4.0f, y, 1.0f);
	::g_pLightManager->theLights[numLight].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].atten.y = 0.1f;
	::g_pLightManager->theLights[numLight].atten.z = 0.1f;

	::g_pLightManager->theLights[numLight].param1.x = 1.0f;    // Spot light (see shader)
	::g_pLightManager->theLights[numLight].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].param1.y = 0.0f;   //  y = inner angle
	::g_pLightManager->theLights[numLight].param1.z = 25.0f;  //  z = outer angle

	::g_pLightManager->theLights[numLight].param2.x = 1.0f;    // Turn on (see shader)

	numLight++;
}

void cLevelGenerator::AddBlueKey(float x, float y)
{
	sMesh* pBlueKey = new sMesh();
	pBlueKey->modelFileName = "assets/LevelModels/BlueKey.ply";
	pBlueKey->positionXYZ = glm::vec3(x, 2.0f, y);
	pBlueKey->rotationEulerXYZ = glm::vec3(-90.0f, 0.0f, 0.0f);
	pBlueKey->uniformScale = 3.0f;
	pBlueKey->bDoNotLight = false;
	pBlueKey->bIsWireframe = false;
	pBlueKey->uniqueFriendlyName = "BlueKey";
	pBlueKey->textures[0] = "Keys.bmp";
	pBlueKey->textures[5] = "Keys_NRM.bmp";
	pBlueKey->hasNormalMap = true;
	pBlueKey->blendRatio[0] = 1.0f;

	::g_vecMeshesToDraw.push_back(pBlueKey);

	//// Set up one of the lights in the scene
	::g_pLightManager->theLights[numLight].position = glm::vec4(x, 4.0f, y, 1.0f);
	::g_pLightManager->theLights[numLight].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].atten.y = 0.1f;
	::g_pLightManager->theLights[numLight].atten.z = 0.1f;

	::g_pLightManager->theLights[numLight].param1.x = 1.0f;    // Spot light (see shader)
	::g_pLightManager->theLights[numLight].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].param1.y = 0.0f;   //  y = inner angle
	::g_pLightManager->theLights[numLight].param1.z = 25.0f;  //  z = outer angle

	::g_pLightManager->theLights[numLight].param2.x = 1.0f;    // Turn on (see shader)

	numLight++;
}

void cLevelGenerator::AddDoor(float x, float y, float roty)
{
	sMesh* pDoor = new sMesh();
	pDoor->modelFileName = "assets/LevelModels/Door.ply";
	pDoor->positionXYZ = glm::vec3(x, 1.0f, y);
	pDoor->rotationEulerXYZ = glm::vec3(0.0f, roty, 0.0f);
	//pDoor->rotationEulerXYZ = glm::vec3(0.0f);
	pDoor->uniformScale = 1.8f;
	pDoor->bDoNotLight = false;
	pDoor->bIsWireframe = false;
	pDoor->uniqueFriendlyName = "Door" + std::to_string(numDoor);
	pDoor->textures[0] = "door.bmp";
	pDoor->textures[5] = "door_NRM.bmp";
	pDoor->hasNormalMap = true;
	pDoor->blendRatio[0] = 1.0f;

	numDoor++;
	::g_vecMeshesToDraw.push_back(pDoor);
	::g_vecDoors.push_back(pDoor);
	//createDynamicWall(PxVec3(pDoor->positionXYZ.x, 5.0f, pDoor->positionXYZ.z), PxVec3(1.0f, 1.0f, 0.5f), pDoor->rotationEulerXYZ);

	g_pPhysicEngine->addTriangleMesh("assets/LevelModels/Door.ply",
		pDoor->positionXYZ,
		pDoor->rotationEulerXYZ,
		pDoor->uniformScale);
}

int numCeiling = 0;
void cLevelGenerator::AddCeiling(float x, float y)
{
	sMesh* pCeiling = new sMesh();
	pCeiling->modelFileName = "assets/LevelModels/Ceiling.ply";
	pCeiling->positionXYZ = glm::vec3(x, 1.0f, y);
	pCeiling->rotationEulerXYZ = glm::vec3(0.0f);
	pCeiling->uniformScale = 1.0f;
	pCeiling->bDoNotLight = false;
	pCeiling->bIsWireframe = false;
	pCeiling->uniqueFriendlyName = "Ceiling" + std::to_string(numCeiling);
	pCeiling->textures[0] = "Ceiling.bmp";
	pCeiling->textures[5] = "Ceiling_NRM.bmp";
	pCeiling->hasNormalMap = true;
	pCeiling->blendRatio[0] = 1.0f;
	pCeiling->bIsCeiling = true;
	numCeiling++;
	::g_vecMeshesToDraw.push_back(pCeiling);
	vecCeilings.push_back(pCeiling);

	g_pPhysicEngine->addTriangleMesh("assets/LevelModels/Ceiling.ply",
		pCeiling->positionXYZ,
		pCeiling->rotationEulerXYZ,
		pCeiling->uniformScale);
}

int numWall = 0;
void cLevelGenerator::AddWall(float x, float y, float roty)
{
	sMesh* pWall = new sMesh();
	pWall->modelFileName = "assets/LevelModels/Wall.ply";
	pWall->positionXYZ = glm::vec3(x, 1.0f, y);
	pWall->rotationEulerXYZ = glm::vec3(0.0f, roty, 0.0f);
	pWall->uniformScale = 1.0f;
	pWall->bDoNotLight = false;
	pWall->bIsWireframe = false;
	pWall->uniqueFriendlyName = "Wall" + std::to_string(numWall);
	pWall->textures[0] = "Wall.bmp";
	pWall->textures[5] = "WallNormal.bmp";
	pWall->hasNormalMap = true;
	pWall->blendRatio[0] = 1.0f;

	numWall++;
	createStaticWall(PxVec3(pWall->positionXYZ.x, 5.0f, pWall->positionXYZ.z), PxVec3(1.0f, 1.5f, 0.5f), pWall->rotationEulerXYZ);
	::g_vecMeshesToDraw.push_back(pWall);

	g_pPhysicEngine->addTriangleMesh("assets/LevelModels/Wall.ply",
		pWall->positionXYZ,
		pWall->rotationEulerXYZ,
		pWall->uniformScale);
} 

float zombieYRotation = 0;
void cLevelGenerator::AddZombie(float x, float y)
{
	Model* ZombieModel = new Model("res/Zombie/Zombie.dae");
	//Model* ZombieModel = new Model("res/Zombie/ZombieHeadless_Rotated_X180.dae");

	ZombieModel->positionXYZ = glm::vec3(x, 1.0f, y);
	ZombieModel->uniformScale = 1.0f;
	ZombieModel->rotationEulerXYZ = glm::vec3(0.0f, zombieYRotation, 0.0f);
	ZombieModel->bIsVisible = true;
	ZombieModel->bIsZombie = true;

	Animation* zombieIdle = new Animation("res/Animations/ZombieIdle.dae", ZombieModel);
	Animation* zombieWalk = new Animation("res/Animations/ZombieWalk.dae", ZombieModel);
	Animation* zombieAttack = new Animation("res/Animations/ZombieAttack.dae", ZombieModel);
	Animation* zombieDie = new Animation("res/Animations/ZombieDeath.dae", ZombieModel);

	Animator* g_pZombieAnimator = new Animator(zombieIdle);

	sModelAnimatorLink* zombieLink = new sModelAnimatorLink();
	zombieLink->model = ZombieModel;
	zombieLink->animator = g_pZombieAnimator;

	vecModelAnimatorLinks.push_back(zombieLink);

	cMasterZombie* pZombie = new cMasterZombie(ZombieModel);
	pZombie->ZombieBeginPlay(ZombieModel, g_pZombieAnimator);

	sAnimationNames* zombieAnimIdle = new sAnimationNames();
	zombieAnimIdle->name = "Idle";
	zombieAnimIdle->animation = zombieIdle;
	pZombie->vecZombieAnims.push_back(zombieAnimIdle);

	sAnimationNames* zombieAnimWalk = new sAnimationNames();
	zombieAnimWalk->name = "Walk";
	zombieAnimWalk->animation = zombieWalk;
	pZombie->vecZombieAnims.push_back(zombieAnimWalk);

	sAnimationNames* zombieAnimAttack = new sAnimationNames();
	zombieAnimAttack->name = "Attack";
	zombieAnimAttack->animation = zombieAttack;
	pZombie->vecZombieAnims.push_back(zombieAnimAttack);

	sAnimationNames* zombieAnimDie = new sAnimationNames();
	zombieAnimDie->name = "Die";
	zombieAnimDie->animation = zombieDie;
	pZombie->vecZombieAnims.push_back(zombieAnimDie);

	AllZombies.push_back(pZombie);

	zombieYRotation += 25.0f;
}

void cLevelGenerator::AddHealth(float x, float y) 
{
	sMesh* pHealth = new sMesh();
	pHealth->modelFileName = "assets/LevelModels/Heart.ply";
	pHealth->positionXYZ = glm::vec3(x, 2.0f, y) + healthOffset;
	pHealth->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pHealth->uniformScale = 0.003f;
	pHealth->bDoNotLight = false;
	pHealth->bIsWireframe = false;
	pHealth->uniqueFriendlyName = "Heart" + std::to_string(numHeart);
	pHealth->textures[0] = "heart.bmp";
	pHealth->blendRatio[0] = 1.0f;

	numHeart++;
	::g_vecMeshesToDraw.push_back(pHealth);

	cHealth* pHealthInteract = new cHealth();
	pHealthInteract->InteractableBeginPlay(pHealth);
	pHealthInteract->pInteractableTrigger->uniformScale = 2.0f;
	g_pVecInteractableHealth.push_back(pHealthInteract);

	//// Set up one of the lights in the scene
	::g_pLightManager->theLights[numLight].position = glm::vec4(x, 4.0f, y, 1.0f) + glm::vec4(healthOffset, 1.0f);
	::g_pLightManager->theLights[numLight].diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].atten.y = 0.1f;
	::g_pLightManager->theLights[numLight].atten.z = 0.1f;

	::g_pLightManager->theLights[numLight].param1.x = 1.0f;    // Spot light (see shader)
	::g_pLightManager->theLights[numLight].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].param1.y = 0.0f;   //  y = inner angle
	::g_pLightManager->theLights[numLight].param1.z = 25.0f;  //  z = outer angle

	::g_pLightManager->theLights[numLight].param2.x = 1.0f;    // Turn on (see shader)

	numLight++;
}

void cLevelGenerator::AddAmmo(float x, float y)
{
	sMesh* pAmmo = new sMesh();
	pAmmo->modelFileName = "assets/LevelModels/ammo.ply";
	pAmmo->positionXYZ = glm::vec3(x, 2.0f, y) + ammoOffset;
	pAmmo->rotationEulerXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	pAmmo->uniformScale = 1.0f;
	pAmmo->bDoNotLight = false;
	pAmmo->bIsWireframe = false;
	pAmmo->uniqueFriendlyName = "Ammo" + std::to_string(numAmmo);
	pAmmo->textures[0] = "ammo.bmp";
	pAmmo->textures[5] = "ammo_NRM.bmp";
	pAmmo->hasNormalMap = true;
	pAmmo->blendRatio[0] = 1.0f;

	numAmmo++;
	::g_vecMeshesToDraw.push_back(pAmmo);

	cAmmo* pAmmoInteract = new cAmmo();
	pAmmoInteract->InteractableBeginPlay(pAmmo);
	pAmmoInteract->pInteractableTrigger->uniformScale = 2.0f;
	g_pVecInteractableAmmo.push_back(pAmmoInteract);

	//// Set up one of the lights in the scene
	::g_pLightManager->theLights[numLight].position = glm::vec4(x, 4.0f, y, 1.0f) + glm::vec4(ammoOffset, 1.0f);
	::g_pLightManager->theLights[numLight].diffuse = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	::g_pLightManager->theLights[numLight].atten.y = 0.1f;
	::g_pLightManager->theLights[numLight].atten.z = 0.1f;

	::g_pLightManager->theLights[numLight].param1.x = 1.0f;    // Spot light (see shader)
	::g_pLightManager->theLights[numLight].direction = glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
	::g_pLightManager->theLights[numLight].param1.y = 0.0f;   //  y = inner angle
	::g_pLightManager->theLights[numLight].param1.z = 25.0f;  //  z = outer angle

	::g_pLightManager->theLights[numLight].param2.x = 1.0f;    // Turn on (see shader)

	numLight++;
}