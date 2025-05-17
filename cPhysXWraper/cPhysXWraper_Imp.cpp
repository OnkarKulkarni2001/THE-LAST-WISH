#include "cPhysXWraper_Imp.h"



//cPhysXWraper_Imp::cPhysXWraper_Imp()
//{
//}
//
//cPhysXWraper_Imp::~cPhysXWraper_Imp()
//{
//}
//
//
//void cPhysXWraper_Imp::update(void)
//{
//
//	return;
//}
//
//void cPhysXWraper_Imp::initPhysics(bool interactive, std::string PVD_Host_Address)
//{
//
//	return;
//}
//
//
//void cPhysXWraper_Imp::getSceneActors(std::vector<cPhysicsObjectTypes>& vecPhysActors)
//{
//	return;
//}
//
//void cPhysXWraper_Imp::cleanupPhysics(bool interactive)
//{
//
//	printf("SnippetHelloWorld done.\n");
//}



// Note that my PhyX is NOT in the project 'include' folder
// C:\PhysX (git)\PhysX\physx\include

// https://nvidia-omniverse.github.io/PhysX/physx/5.3.1/docs/BuildingWithPhysX.html

#include "PxPhysicsAPI.h"
#include "cooking/PxCooking.h"
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> // For glm::quat
#include <glm/gtx/euler_angles.hpp> // For eulerAngleYXZ
#include <vector>

using namespace physx;

PxDefaultAllocator		gAllocator;
PxDefaultErrorCallback	gErrorCallback;
PxFoundation* gFoundation = NULL;
PxPhysics* gPhysics = NULL;
PxDefaultCpuDispatcher* gDispatcher = NULL;
PxScene* gScene = NULL;
PxMaterial* gMaterial = NULL;
PxPvd* gPvd = NULL;
PxRigidStatic* doorActor = NULL;

std::vector<PxRigidDynamic*> g_pVecZombieDynamic;

PxRigidDynamic* gPlayerBody = nullptr;

// These are taken from the SnippedHelloWorld example.
// We are only using these now to make sure that the PhysX stuff is up and running.
// 
// i.e. we will be deleting these once we are sure that PhysX is working OK
static PxReal stackZ = 10.0f;
void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent);
PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry,
	const PxVec3& velocity /*= PxVec3(0)*/);


cPhysXWraper_Imp::cPhysXWraper_Imp()
{



}

cPhysXWraper_Imp::~cPhysXWraper_Imp()
{



}


void cPhysXWraper_Imp::update(void)
{
	// From renderCallback() in SnippetHelloWorld

	//stepPhysics(true);
	gScene->simulate(1.0f / 60.0f);
	gScene->fetchResults(true);

	// Don't need this - it's for the SnippedHelloWorld GLUT rendering
	//		Snippets::startRender(sCamera);

	PxScene* scene;
	PxGetPhysics().getScenes(&scene, 1);
	PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
	if (nbActors)
	{
		std::vector<PxRigidActor*> actors(nbActors);
		scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
		//			Snippets::renderActors(&actors[0], static_cast<PxU32>(actors.size()), true);
	}

	// Don't need this - it's for the SnippedHelloWorld GLUT rendering
	//		Snippets::finishRender();

	return;
}

void cPhysXWraper_Imp::initPhysics(bool interactive, std::string PVD_Host_Address)
{
	// Taken from the SnippetHelloWorld.cpp file:

	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	// This is connecting to the visual debugger...
	gPvd = PxCreatePvd(*gFoundation);
	//	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 10);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_Host_Address.c_str(), 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	// This creates the 'world' that all the physics things are in
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);


// **** 
	//{
	//	// Some code to create a sphere
	//	PxShape* sphereShape = gPhysics->createShape(PxSphereGeometry(3.0f), *gMaterial);
	//	PxTransform localTm(PxVec3(10.0f, 50.0f, 0.0f));
	//	PxRigidDynamic* body = gPhysics->createRigidDynamic(PxTransform(localTm));
	//	body->attachShape(*sphereShape);
	//	PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
	//	gScene->addActor(*body);
	//}
// **** 



// ********************************************************
// This block of code is from the hello world PhyX example.

//	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, -10, 0, 0), *gMaterial);
//	gScene->addActor(*groundPlane);
//
//
//	for (PxU32 i = 0; i < 5; i++)
//		createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f);
////
//	if (!interactive)
//		createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10), PxVec3(0, -50, -100));
//// ********************************************************



	return;
}

// HACK: Remove this later
void cPhysXWraper_Imp::HACK_Snippet_ShootBall(void)
{
	// In the original snippet example, it had this code:
	//void keyPress(unsigned char key, const PxTransform & camera)
	//{
	//	switch (toupper(key))
	//	{
	//	case 'B':	createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f);						break;
	//	case ' ':	createDynamic(camera, PxSphereGeometry(3.0f), camera.rotate(PxVec3(0, 0, -1)) * 200);	break;
	//	}
	//}
	createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10), PxVec3(0, -50, -100));

	return;
}

void cPhysXWraper_Imp::getSceneActors(std::vector<cPhysicsObjectTypes>& vecPhysActors)
{

	vecPhysActors.clear();

	// Taken from the Snippet::HelloWorld
	PxScene* scene;
	PxGetPhysics().getScenes(&scene, 1);
	PxU32 nbActors = scene->getNbActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC);
	if (nbActors)
	{
		std::vector<PxRigidActor*> actors(nbActors);
		scene->getActors(PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC, reinterpret_cast<PxActor**>(&actors[0]), nbActors);
		//		Snippets::renderActors(&actors[0], static_cast<PxU32>(actors.size()), true);

					// This number is taken from the SnippetHelloWorld
				//	and is used in the render call. 
		const unsigned int MAX_NUM_ACTOR_SHAPES = 128;

		PxShape* shapes[MAX_NUM_ACTOR_SHAPES];

		unsigned int numActors = static_cast<unsigned int>(actors.size());

		for (PxU32 i = 0; i < numActors; i++)
		{
			const PxU32 nbShapes = actors[i]->getNbShapes();
			PX_ASSERT(nbShapes <= MAX_NUM_ACTOR_SHAPES);
			actors[i]->getShapes(shapes, nbShapes);

			//			bool sleeping;
			//			if (changeColorForSleepingActors)
			//				sleeping = actors[i]->is<PxRigidDynamic>() ? actors[i]->is<PxRigidDynamic>()->isSleeping() : false;
			//			else
			//				sleeping = false;

			for (PxU32 j = 0; j < nbShapes; j++)
			{
				const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));
				const PxGeometry& geom = shapes[j]->getGeometry();


				//				if (sleeping)
				//				{
				//					const PxVec3 darkColor = color * 0.25f;
				//					glColor4f(darkColor.x, darkColor.y, darkColor.z, 1.0f);
				//				}
				//				else
				//					glColor4f(color.x, color.y, color.z, 1.0f);

				//				renderGeometry(geom);

				switch (geom.getType())
				{
				case PxGeometryType::eBOX:
				{
					const PxBoxGeometry& boxGeom = static_cast<const PxBoxGeometry&>(geom);
					//						glScalef(boxGeom.halfExtents.x, boxGeom.halfExtents.y, boxGeom.halfExtents.y);
					//						glutSolidCube(2);

											// 		BOX,		// Params: "half-extents": x,y,z 
					cPhysicsObjectTypes box;
					box.shapeType = cPhysicsObjectTypes::BOX;
					box.map_ParamToValue["half-extents"] = glm::vec4(boxGeom.halfExtents.x,
						boxGeom.halfExtents.y,
						boxGeom.halfExtents.z,
						1.0f);

					const PxMat44 shapePose(PxShapeExt::getGlobalPose(*shapes[j], *actors[i]));

					// They are using glu in the Snippets::HelloWorld
					//	glMultMatrixf(&shapePose.column0.x);

					// From https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.20.pdf
					// Section: 5.4.2 Vector and Matrix Constructors
					// (Page 78)
					// 
					//	mat4(float, float, float, float, // first column
					//	     float, float, float, float, // second column
					//	     float, float, float, float, // third column
					//	     float, float, float, float); // fourth column

					box.matModel = glm::mat4(shapePose.column0.x, shapePose.column0.y, shapePose.column0.z, shapePose.column0.w,
						shapePose.column1.x, shapePose.column1.y, shapePose.column1.z, shapePose.column1.w,
						shapePose.column2.x, shapePose.column2.y, shapePose.column2.z, shapePose.column2.w,
						shapePose.column3.x, shapePose.column3.y, shapePose.column3.z, shapePose.column3.w);

					// Can get other stuff, too
					// shapes[j]->getDensityForFluid(...)
					//shapes[j]->getMaterials(

					vecPhysActors.push_back(box);

				}
				break;

				case PxGeometryType::eSPHERE:
				{
					const PxSphereGeometry& sphereGeom = static_cast<const PxSphereGeometry&>(geom);
					//						glutSolidSphere(GLdouble(sphereGeom.radius), 10, 10);

											// SPHERE,		// Params: "radius": x

					cPhysicsObjectTypes sphere;
					sphere.shapeType = cPhysicsObjectTypes::SPHERE;
					sphere.map_ParamToValue["radius"] = glm::vec4(sphereGeom.radius, 0.0f, 0.0f, 1.0f);

					sphere.matModel = glm::mat4(shapePose.column0.x, shapePose.column0.y, shapePose.column0.z, shapePose.column0.w,
						shapePose.column1.x, shapePose.column1.y, shapePose.column1.z, shapePose.column1.w,
						shapePose.column2.x, shapePose.column2.y, shapePose.column2.z, shapePose.column2.w,
						shapePose.column3.x, shapePose.column3.y, shapePose.column3.z, shapePose.column3.w);


					// Can get other stuff, too
					// shapes[j]->getDensityForFluid(...)
					//shapes[j]->getMaterials(

					vecPhysActors.push_back(sphere);

				}
				break;
				case PxGeometryType::eCAPSULE:
				{
					const PxCapsuleGeometry& capsuleGeom = static_cast<const PxCapsuleGeometry&>(geom);

					// CAPSULE,    // Params: "radius": x, "halfHeight": y

					cPhysicsObjectTypes capsule;
					capsule.shapeType = cPhysicsObjectTypes::CAPSULE;
					capsule.map_ParamToValue["radius"] = glm::vec4(capsuleGeom.radius, 0.0f, 0.0f, 1.0f);
					capsule.map_ParamToValue["halfHeight"] = glm::vec4(capsuleGeom.halfHeight, 0.0f, 0.0f, 1.0f);

					capsule.matModel = glm::mat4(shapePose.column0.x, shapePose.column0.y, shapePose.column0.z, shapePose.column0.w,
						shapePose.column1.x, shapePose.column1.y, shapePose.column1.z, shapePose.column1.w,
						shapePose.column2.x, shapePose.column2.y, shapePose.column2.z, shapePose.column2.w,
						shapePose.column3.x, shapePose.column3.y, shapePose.column3.z, shapePose.column3.w);

					// Can get other stuff, too
					// shapes[j]->getDensityForFluid(...)
					// shapes[j]->getMaterials(...)

					vecPhysActors.push_back(capsule);
				}
				break;

				}

			}//for (PxU32 j = 0; j < nbShapes;
		}//for (PxU32 i = 0; i < numActors; i++)

	}//if (nbActors)

	return;
}

void cPhysXWraper_Imp::cleanupPhysics(bool interactive)
{
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PX_RELEASE(gPhysics);
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();	gPvd = NULL;
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);

	printf("SnippetHelloWorld done.\n");
}

void createPlayerDynamic(PxVec3 playerPosition, PxF32 radius, PxF32 halfHeight)
{
	PxShape* playerShape = gPhysics->createShape(PxCapsuleGeometry(radius, halfHeight), *gMaterial);

	PxTransform playerTm(playerPosition);
	gPlayerBody = gPhysics->createRigidDynamic(playerTm);
	gPlayerBody->attachShape(*playerShape);
	PxRigidBodyExt::updateMassAndInertia(*gPlayerBody, 10.0f);
	gScene->addActor(*gPlayerBody);

	playerShape->release();
}

void createZombieDynamic(PxVec3 zombiePosition, PxF32 radius, PxF32 halfHeight)
{
	PxShape* playerShape = gPhysics->createShape(PxCapsuleGeometry(radius, halfHeight), *gMaterial);

	PxTransform playerTm(zombiePosition);
	PxRigidDynamic* zombieDynamic = gPhysics->createRigidDynamic(playerTm);
	zombieDynamic->attachShape(*playerShape);
	PxRigidBodyExt::updateMassAndInertia(*zombieDynamic, 10.0f);
	gScene->addActor(*zombieDynamic);

	g_pVecZombieDynamic.push_back(zombieDynamic);
	playerShape->release();
}

// These are taken from the SnippedHelloWorld example.
// We are only using these now to make sure that the PhysX stuff is up and running.
void createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for (PxU32 i = 0; i < size; i++)
	{
		for (PxU32 j = 0; j < size - i; j++)
		{
			PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 10.0f);
			gScene->addActor(*body);
		}
	}
	shape->release();
}

void createStaticWall(const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation)
{
	// Create a material (friction, restitution)
	PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// Define the shape of the wall (box)
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtents), *material);

	// Define transform with rotation
	PxTransform transform(position, PxQuat(glm::radians(rotation.y), PxVec3(0, 1, 0)));

	// Create static rigid body (it won’t move)
	doorActor = gPhysics->createRigidStatic(transform);
	doorActor->attachShape(*shape);

	// Add actor to the scene
	gScene->addActor(*doorActor);

	// Release shape (safe to release after attaching)
	shape->release();
}

void createDynamicWall(const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation)
{
	// Create material
	PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	// Create shape
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtents), *material);

	// Define transform with rotation
	PxTransform transform(position, PxQuat(glm::radians(rotation.y), PxVec3(0, 1, 0)));

	// Create dynamic rigid body
	PxRigidDynamic* dynamicWallActor = gPhysics->createRigidDynamic(transform);
	dynamicWallActor->attachShape(*shape);

	// Set mass and inertia
	PxRigidBodyExt::updateMassAndInertia(*dynamicWallActor, 10.0f); // Adjust mass as needed

	// Lock linear movement (position) in all axes
	dynamicWallActor->setRigidDynamicLockFlags(
		PxRigidDynamicLockFlag::eLOCK_LINEAR_X |
		PxRigidDynamicLockFlag::eLOCK_LINEAR_Y |
		PxRigidDynamicLockFlag::eLOCK_LINEAR_Z |

		// Lock rotation in X and Z axes, allow only Y rotation
		PxRigidDynamicLockFlag::eLOCK_ANGULAR_X |
		PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z
	);

	// Add to the scene
	gScene->addActor(*dynamicWallActor);

	// Release shape after attaching
	shape->release();
}


PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry,
	const PxVec3& velocity = PxVec3(0))
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	gScene->addActor(*dynamic);
	return dynamic;
}
