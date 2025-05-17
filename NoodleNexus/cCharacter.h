#pragma once

#include "../Animation/Animator.h"
#include "../Animation/Animation.h"
#include "../Animation/Model.h"
#include <unordered_map>


class cCharacter
{
public:
	cCharacter(Model& model);
	~cCharacter();

	//void SetAnimation(Animation* animation);
	void AddAnimation(std::string name, Animation* animation);
	void SetAnimation(std::string);
	void PlayAnimation(Animation* animation, float animationSpeed = 1.0f);

	Model* characterModel;
	std::unordered_map<std::string, Animation*> animations;
	std::string currentAnimationName;
	Animation* currentAnimation;
};