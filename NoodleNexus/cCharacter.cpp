#include "cCharacter.h"
extern Animator* g_pAnimator;

//cCharacter::cCharacter(Model& model)
//{
//	this->characterModel = &model;
//}
//
//cCharacter::~cCharacter()
//{
//}
//
////void cCharacter::SetAnimation(Animation* animation)
////{
////	currentAnimation = *animation;
////}
//
//void cCharacter::AddAnimation(std::string name, Animation* animation)
//{
//	animations[name] = animation;
//}
//
//void cCharacter::SetAnimation(std::string name)
//{
//    if (animations.find(name) != animations.end())
//    {
//        if (currentAnimationName != name)
//        {
//            currentAnimation = animations[name];
//            currentAnimationName = name;
//            currentTime = 0.0f;
//        }
//    }
//    else
//    {
//        std::cout << "This animatiopn is already added\n";
//    }
//}
//
//void cCharacter::PlayAnimation(Animation* animation, float animationSpeed)
//{
//	if (g_pAnimator->GetCurrentAnimation() != animation) {
//		currentAnimation = animation;
//		g_pAnimator->SetCurrentAnimation(animation);
//		g_pAnimator->ResetCurrentTime();
//		g_pAnimator->SpeedUpAnimation(animationSpeed);
//	}
//}

