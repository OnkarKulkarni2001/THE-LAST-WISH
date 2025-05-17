#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/common.hpp>


class cLevelGenerator
{
public:
	cLevelGenerator();
	~cLevelGenerator();

	void SetupMap(int x, int y);
	void ReadMap(std::string path);
	void GenerateMap();


	int xRows;
	int yColoums;
	char** mapData;
	int numHeart = 0;
	int numAmmo = 0;
	void AddFloor(float x, float y);
	void AddRedKey(float x, float y);
	void AddGreenKey(float x, float y);
	void RotateKeys(float deltaTime);
	void GenerateZombies(std::string path);
	void GenerateKeysOnMap(std::string path);
	void AddYellowKey(float x, float y);
	void AddBlueKey(float x, float y);
	void AddDoor(float x, float y, float roty);
	void AddCeiling(float x, float y);
	void AddWall(float x, float y, float roty);
	void AddZombie(float x, float y);
	void AddHealth(float x, float y);
	void AddAmmo(float x, float y);
	void GenerateHealthAndAmmo(std::string path);

	void AddLight(float x, float y, float roty);
	void GenerateLights(std::string path);
};
