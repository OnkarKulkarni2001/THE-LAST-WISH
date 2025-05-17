//#define GLAD_GL_IMPLEMENTATION
//#include <glad/glad.h>
//
//#define GLFW_INCLUDE_NONE
//#include <GLFW/glfw3.h>
#include "GLCommon.h"


//#include "linmath.h"
//#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
//#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <iostream>     // "input output" stream
#include <fstream>      // "file" stream
#include <sstream>      // "string" stream ("string builder" in Java c#, etc.)
#include <string>
#include <vector>

//void ReadPlyModelFromFile(std::string plyFileName);
#include "PlyFileLoaders.h"
#include "Basic_Shader_Manager/cShaderManager.h"
#include "sMesh.h"
#include "cVAOManager/cVAOManager.h"
#include "sharedThings.h"       // Fly camera
#include "cPhysics.h"
#include "cLightManager.h"
#include <windows.h>    // Includes ALL of windows... MessageBox
#include "cLightHelper/cLightHelper.h"
//
#include "cBasicTextureManager/cBasicTextureManager.h"

#include "cLowPassFilter.h"

#include "cTerrainPathChooser.h"

// Frame Buffer Object (FBO)
#include "cFBO/cFBO_RGB_depth.h"
// Deferred render FBO
#include "cFBO/cFBO_deferred.h"
#include "cDepthCubeMap.h"
#include "cViperFlagConnector.h"

#include "PhysXWraper/cPhysXWraper.h"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

//#include "cParticleEmitter.h"
#include "cParticleEmitter_2.h"
//#include "cParticleEmmiterOwn.h"

#include "Animation/Model.h"
#include "cCharacter.h"
#include "Animation/Animator.h"
#include "Animation/Animation.h"
#include "cMasterInteractable.h"
#include "cDoor.h"
#include "cPlayer.h"
#include "cTimelineManager.h"
#include "cGun.h"
#include "cKey.h"
#include "cLevelGenerator.h"
#include "Health.h"
#include "cAmmo.h"
#include "cBullet.h"
#include "cSoundManager.h"


float frontDistance = 0.0f;
float backDistance = 0.0f;
float rightDistance = 0.0f;
float leftDistance = 0.0f;
float botDistance;
float downDistance = 0.0f;
float collisionThreshold = 3.0f;

float g_orthoSize = 10.0f;
double deltaX, deltaY;
extern bool bUseThreading;

std::vector<sModelAnimatorLink*> vecModelAnimatorLinks;
std::vector<cMasterZombie*> AllZombies;
extern std::vector<cBullet*> vecBullets;
//cParticleEmmiterOwn* g_myParticleEmmiter = NULL;
cLevelGenerator* pLevelGenerator = NULL;

extern std::vector<sMesh*> g_vecDoors;
std::vector<cDoor*> g_pVecInteractableDoors;
std::vector<cKey*> g_pVecInteractableKeys;
std::vector<cHealth*> g_pVecInteractableHealth;
std::vector<cAmmo*> g_pVecInteractableAmmo;

std::vector<Model*> vecAnimatedModels;
Animator* g_pAnimator = nullptr;
double deltaTime = 0.0f;
//cCharacter* g_pCharacter = nullptr;

// Deferred rendering Geometry "G" buffer
cFBO_deferred* g_pFBO_G_Buffer = NULL;
cDepthCubeMap* g_pShadowCubeMapFBO = NULL;

GLuint program = NULL;
GLuint shadowShaderProgram = NULL;

//const unsigned int MAX_NUMBER_OF_MESHES = 1000;
//unsigned int g_NumberOfMeshesToDraw;
//sMesh* g_myMeshes[MAX_NUMBER_OF_MESHES] = { 0 };    // All zeros
GLFWwindow* window = nullptr;
void RenderImGui();
void DrawCrosshair(int screenWidth, int screenHeight);
void ShowControlsUIBottomRight();
std::vector<sMesh*> g_vecMeshesToDraw;

cPhysics* g_pPhysicEngine = NULL;
cPhysXWraper* g_pPhysX = NULL;


cTimelineManager* pTimelineManager = NULL;

cSoundManager* g_pSoundManager = NULL;

//My Global Stuff
cPlayer* pPlayerReference = NULL;
bool bIsAiming = false;
float FoV = 0.6f;



// This loads the 3D models for drawing, etc.
cVAOManager* g_pMeshManager = NULL;

cBasicTextureManager* g_pTextures = NULL;

cCommandGroup* g_pCommandDirector = NULL;
cCommandFactory* g_pCommandFactory = NULL;

cTerrainPathChooser* g_pTerrainPathChooser = NULL;

extern cViperFlagConnector* g_pViperFlagConnector;
extern cLightManager* g_pLightManager;
void FBOInit(cFBO_RGB_depth& FBO_View);

//cParticleEmitter* g_pParticles = NULL;
cParticleEmitter_2* g_pParticles = NULL;

void InitDebugLineVAO(GLuint shaderProgramID);
void DrawDebugLineModern(const glm::vec3& start, const glm::vec3& end,
    const glm::mat4& view, const glm::mat4& projection,
    GLuint shaderProgram);

glm::vec3 GetCrosshairWorldPos(float screenWidth, float screenHeight, const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix, std::vector<cMasterZombie*> AllZombies, int numZombies);

void AddZombie(float x, float y);
//cLightManager* g_pLightManager = NULL;
void createStaticWall(PxRigidStatic*& rigidDoor, const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation);
void AddModelsToScene(cVAOManager* pMeshManager, GLuint shaderProgram);
void RenderAllAnimated(GLuint& program, glm::mat4& matProjection, glm::mat4& matView, float deltaTime);
glm::vec3 screenToWorldBottomRight(glm::mat4 proj, glm::mat4 view, float screenWidth, float screenHeight);
glm::vec3 WorldToScreen(glm::vec3& worldPos, glm::mat4& view, glm::mat4& proj, int screenWidth, int screenHeight);
glm::vec3 ScreenToWorld(const glm::vec3& screenPos, const glm::mat4& view, const glm::mat4& proj, int screenWidth, int screenHeight);
void RenderMinimap(GLuint texture, int screenWidth, int screenHeight);
void DrawInteractUI(glm::vec3& objectPos, glm::vec3& playerPos,
    glm::mat4& view, glm::mat4& proj,
    int screenWidth, int screenHeight);
glm::vec3 getCameraLookDirection();

//void DrawMesh(sMesh* pCurMesh, GLuint program, bool SetTexturesFromMeshInfo = true);
// Now we pass the original (parent) matrix.
// We can also pass this matrix instead of the position, orientation, etc.
void DrawMesh(sMesh* pCurMesh, glm::mat4 matModel, GLuint program, bool SetTexturesFromMeshInfo = true);

//glm::vec3 cameraEye = glm::vec3(0.0, 0.0, 4.0f);

void RenderSceneDepth(GLuint shadowShaderProgram);
void RenderScene(
    GLuint program,
    glm::mat4 matProjection,
    glm::mat4 matView,
    float ratio,
    glm::vec3 eyeLocation);

void DrawZombieShooterHUD(float deltaTime);

void RenderFBOTextureSingleMesh(sMesh* pFBOTextureMesh, cFBO_RGB_depth& FBO_View, GLuint program, glm::mat4& matProjection, glm::mat4& matView, float ratio);

// This is the function that Lua will call when 
//void g_Lua_AddSerialCommand(std::string theCommandText)
int g_Lua_AddSerialCommand(lua_State* L)
{
//    std::cout << "**************************" << std::endl;
//    std::cout << "g_Lua_AddSerialCommand() called" << std::endl;
//    std::cout << "**************************" << std::endl;
    // AddSerialCommand() has been called
    // eg: AddSerialCommand('New_Viper_Player', -50.0, 15.0, 30.0, 5.0)

    std::string objectFriendlyName = lua_tostring(L, 1);      // 'New_Viper_Player'
    float x = (float)lua_tonumber(L, 2);                   // -50.0
    float y = (float)lua_tonumber(L, 3);                   // 15.0
    float z = (float)lua_tonumber(L, 4);                   // 30.0
    float timeSeconds = (float)lua_tonumber(L, 5);                   // 5.0

    std::vector<std::string> vecCommandDetails;
    vecCommandDetails.push_back(objectFriendlyName);    // Object command controls
    vecCommandDetails.push_back(::g_floatToString(x));
    vecCommandDetails.push_back(::g_floatToString(y));
    vecCommandDetails.push_back(::g_floatToString(z));
    vecCommandDetails.push_back(::g_floatToString(timeSeconds));

    iCommand* pMoveViper = ::g_pCommandFactory->pCreateCommandObject(
        "Move Relative ConstVelocity+Time", vecCommandDetails);

    ::g_pCommandDirector->addSerial(pMoveViper);

    // We'll return some value to indicate if the command worked or not
    // Here, we'll push "true" if it worked
    lua_pushboolean(L, true);
    // return 1 because we pushed 1 thing onto the stack
    return 1;
}

void FBOBind(cFBO_RGB_depth& FBO_View, float ratio, GLuint program, glm::vec3 eyeLocation, glm::vec3 targetLocation, float ortho);

//void FBOBind(cFBO_RGB_depth& FBO_View, float ratio, GLuint program, glm::vec3 eyeLocation, glm::vec3 targetLocation);
//void FBOBind(cFBO_RGB_depth& FBO_View, float ratio, GLuint program, glm::vec3 eyeLocation, glm::vec3 targetLocation) {
//
//    //sMesh* skySphere = g_pFindMeshByFriendlyName("SkySphere");
//    //skySphere->bIsVisible = false;
//
//    glm::mat4 matProjection = glm::mat4(1.0f);
//    glm::mat4 matView = glm::mat4(1.0f);
//    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
//
//    // **************************************************
//    // 
//    // RENDER from the inside of the warehouse
//
//
//    // Point output to the off-screen FBO
//    glBindFramebuffer(GL_FRAMEBUFFER, FBO_View.ID);
//
//    // These will ONLY work on the default framebuffer
//       // glViewport(0, 0, width, height);
//       //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//       // FBO_WarehouseView.clearColourBuffer(0);
//    FBO_View.clearBuffers(true, true);
//
//
//    matProjection = glm::perspective(0.6f,
//        ratio,
//        0.1f,
//        1000.0f);
//
//
//    matView = glm::lookAt(eyeLocation, targetLocation, glm::vec3(0.0f, 1.0f, 0.0f));
//    glUseProgram(program);
//    //std::cout << "Eye: " << eyeLocation.x << ", " << eyeLocation.y << ", " << eyeLocation.z << std::endl;
//    //std::cout << "Target: " << targetLocation.x << ", " << targetLocation.y << ", " << targetLocation.z << std::endl;
//
//    RenderScene(program, matProjection, matView, ratio, eyeLocation);
//    //skySphere->bIsVisible = true;1
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}

void makeFBOTexture(cFBO_RGB_depth& FBO_ScreenTest1, cFBO_RGB_depth& FBO_ScreenTest2,
    cFBO_RGB_depth& FBO_ScreenTest3, cFBO_RGB_depth& FBO_ScreenTest4,
    sMesh* pMesh1, sMesh* pMesh2, sMesh* pMesh3, sMesh* pMesh4,
    glm::mat4& matProjection, glm::mat4& matView, float ratio)
{
    if (pMesh1 && pMesh2 && pMesh3 && pMesh4)
    {
        RenderScene(program, matProjection, matView, ratio, ::g_pFlyCamera->getEyeLocation());

        pMesh1->bIsVisible = true;
        pMesh2->bIsVisible = true;
        pMesh3->bIsVisible = true;
        pMesh4->bIsVisible = true;

        GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        GLint matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        // Connect texture unit #0 to the offscreen FBO
        glActiveTexture(GL_TEXTURE0);

        // The colour texture inside the FBO is just a regular colour texture.
        // There's nothing special about it.
        glBindTexture(GL_TEXTURE_2D, FBO_ScreenTest1.colourTexture_0_ID);
        //glBindTexture(GL_TEXTURE_2D, ::g_pTextures->getTextureIDFromName("dua-lipa-promo.bmp"));

        GLint texture01_UL = glGetUniformLocation(program, "texture00");
        glUniform1i(texture01_UL, 0);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here

        GLint texRatio_0_to_3_UL = glGetUniformLocation(program, "texRatio_0_to_3");
        glUniform4f(texRatio_0_to_3_UL,
            1.0f,
            0.0f,
            0.0f,
            0.0f);
        //GLint b_Is_FBO_Texture_UL = glGetUniformLocation(program, "b_Is_FBO_Texture");

        //glUniform1f(b_Is_FBO_Texture_UL, (GLfloat)GL_TRUE);

        //GLint bUseTextureAsColour_UL = glGetUniformLocation(program, "bUseTextureAsColour");
        //glUniform1f(bUseTextureAsColour_UL, (GLfloat)GL_TRUE);


        DrawMesh(pMesh1, program, false);
        pMesh1->bIsVisible = false;
        //glUniform1f(b_Is_FBO_Texture_UL, (GLfloat)GL_FALSE);
        //glUniform1f(bUseTextureAsColour_UL, (GLfloat)GL_FALSE);

        //-------------------------------------------------------------------------------------

        matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        // Connect texture unit #0 to the offscreen FBO
        glActiveTexture(GL_TEXTURE0);

        // The colour texture inside the FBO is just a regular colour texture.
        // There's nothing special about it.
        glBindTexture(GL_TEXTURE_2D, FBO_ScreenTest2.colourTexture_0_ID);
        //glBindTexture(GL_TEXTURE_2D, ::g_pTextures->getTextureIDFromName("dua-lipa-promo.bmp"));

        texture01_UL = glGetUniformLocation(program, "texture00");
        glUniform1i(texture01_UL, 0);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here

        texRatio_0_to_3_UL = glGetUniformLocation(program, "texRatio_0_to_3");
        glUniform4f(texRatio_0_to_3_UL,
            1.0f,
            0.0f,
            0.0f,
            0.0f);
        GLint b_Is_FBO_Texture_UL = glGetUniformLocation(program, "b_Is_FBO_Texture");
        glUniform1f(b_Is_FBO_Texture_UL, (GLfloat)GL_TRUE);

        DrawMesh(pMesh2, program, false);
        pMesh2->bIsVisible = false;
        glUniform1f(b_Is_FBO_Texture_UL, (GLfloat)GL_FALSE);

        //-------------------------------------------------------------------------------------

        matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        // Connect texture unit #0 to the offscreen FBO
        glActiveTexture(GL_TEXTURE0);

        // The colour texture inside the FBO is just a regular colour texture.
        // There's nothing special about it.
        glBindTexture(GL_TEXTURE_2D, FBO_ScreenTest3.colourTexture_0_ID);
        //glBindTexture(GL_TEXTURE_2D, ::g_pTextures->getTextureIDFromName("dua-lipa-promo.bmp"));

        texture01_UL = glGetUniformLocation(program, "texture00");
        glUniform1i(texture01_UL, 0);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here

        texRatio_0_to_3_UL = glGetUniformLocation(program, "texRatio_0_to_3");
        glUniform4f(texRatio_0_to_3_UL,
            1.0f,
            0.0f,
            0.0f,
            0.0f);

        DrawMesh(pMesh3, program, false);
        pMesh3->bIsVisible = false;

        //-------------------------------------------------------------------------------------



        matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        // Connect texture unit #0 to the offscreen FBO
        glActiveTexture(GL_TEXTURE0);

        // The colour texture inside the FBO is just a regular colour texture.
        // There's nothing special about it.
        glBindTexture(GL_TEXTURE_2D, FBO_ScreenTest4.colourTexture_0_ID);
        //glBindTexture(GL_TEXTURE_2D, ::g_pTextures->getTextureIDFromName("dua-lipa-promo.bmp"));

        texture01_UL = glGetUniformLocation(program, "texture00");
        glUniform1i(texture01_UL, 0);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here

        texRatio_0_to_3_UL = glGetUniformLocation(program, "texRatio_0_to_3");
        glUniform4f(texRatio_0_to_3_UL,
            1.0f,
            0.0f,
            0.0f,
            0.0f);


        GLint b_is_Second_Screen_UL = glGetUniformLocation(program, "bIsSecondScreen");
        glUniform1f(b_is_Second_Screen_UL, (GLfloat)GL_TRUE);

        DrawMesh(pMesh4, program, false);
        pMesh4->bIsVisible = false;
        glUniform1f(b_is_Second_Screen_UL, (GLfloat)GL_FALSE);

    }
    //  RenderScene(program, matProjection, matView, ratio, ::g_pFlyCamera->getEyeLocation());

}


Animation* GetAnimationByName(std::string name, std::vector<sAnimationNames*> vecAnimNames)
{
    for (sAnimationNames* anim : vecAnimNames) {
        if (anim->name == name) {
            return anim->animation;
        }
    }
    return nullptr;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

bool isControlDown(GLFWwindow* window);
//{
//    if ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) ||
//        (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS))
//    {
//        return true;
//    }
//    return false;
//}

// START OF: TANK GAME
//#include "iTank.h"
//#include "cTank.h"
//#include "cSuperTank.h"
//#include "cTankFactory.h"
#include "cTankBuilder.h"
#include "cArena.h"
void SetUpTankGame(void);
void TankStepFrame(double timeStep);
std::vector< iTank* > g_vecTheTanks;
cArena* g_pTankArena = NULL;
sMesh* g_pTankModel = NULL;

// END OF: TANK GAME





void ConsoleStuff(void);

// https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats
float g_getRandomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

glm::vec3 g_getRandom_vec3(glm::vec3 min, glm::vec3 max)
{
    return glm::vec3(
        ::g_getRandomFloat(min.x, max.x),
        ::g_getRandomFloat(min.y, max.y),
        ::g_getRandomFloat(min.z, max.z));
}

std::string g_getStringVec3(glm::vec3 theVec3)
{
    std::stringstream ssVec;
    ssVec << "(" << theVec3.x << ", " << theVec3.y << ", " << theVec3.z << ")";
    return ssVec.str();
}

// Returns NULL if NOT found
sMesh* g_pFindMeshByFriendlyName(std::string theNameToFind)
{
    for (unsigned int index = 0; index != ::g_vecMeshesToDraw.size(); index++)
    {
        if (::g_vecMeshesToDraw[index]->uniqueFriendlyName == theNameToFind)
        {
            return ::g_vecMeshesToDraw[index];
        }
    }
    // Didn't find it
    return NULL;
}

void AABBOctTree(void);

float LineTraceForward(GLuint program, float maxDistance)
{
    cPhysics::sLine LASERbeam;
    glm::vec3 LASERbeam_Offset = glm::vec3(0.0f, -2.0f, 0.0f);
    float distance;

    if (::g_bShowLASERBeam)
    {
        // The fly camera is always "looking at" something 1.0 unit away
        glm::vec3 cameraDirection = ::g_pFlyCamera->getTargetRelativeToCamera();     //0,0.1,0.9

        LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();

        // Move the LASER below the camera
        LASERbeam.startXYZ += LASERbeam_Offset;
        glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

        // Is the LASER less than 500 units long?
        // (is the last LAZER ball we drew beyond 500 units form the camera?)
        while (glm::distance(::g_pFlyCamera->getEyeLocation(), LASER_ball_location) < maxDistance)
        {
            // Move the next ball 0.1 times the normalized camera direction
            LASER_ball_location += (cameraDirection * 0.10f);
            //DrawDebugSphere(LASER_ball_location, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.05f, program);
        }
        LASERbeam.endXYZ = LASER_ball_location;
    }

    float shortestDistance = maxDistance;

    //Draw the end of this LASER beam
    //DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

    // Now draw a different coloured ball wherever we get a collision with a triangle
    std::vector<cPhysics::sCollision_RayTriangleInMesh> vec_RayTriangle_Collisions;
    ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, vec_RayTriangle_Collisions, false);

    glm::vec4 triColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    float triangleSize = 0.25f;

    for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions.begin();
        itTriList != vec_RayTriangle_Collisions.end(); itTriList++)
    {
        for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles.begin();
            itTri != itTriList->vecTriangles.end(); itTri++)
        {
            //DrawDebugSphere(itTri->intersectionPoint, triColour, triangleSize, program);
            triColour.r -= 0.1f;
            triColour.g -= 0.1f;
            triColour.b += 0.2f;
            triangleSize *= 1.25f;
            distance = glm::distance(itTri->intersectionPoint, ::g_pFlyCamera->getEyeLocation());
            if (distance < shortestDistance)
            {
                shortestDistance = distance;
            }
        }

    }
    return shortestDistance;
}

float LineTrace(glm::vec3 cameraDirection, GLuint program, float maxDistance)
{
    cPhysics::sLine LASERbeam;
    glm::vec3 LASERbeam_Offset = glm::vec3(0.0);
    float distance{};

    if (::g_bShowLASERBeam)
    {
        LASERbeam.startXYZ = ::g_pFlyCamera->getEyeLocation();

        // Move the LASER below the camera
        LASERbeam.startXYZ += LASERbeam_Offset;
        glm::vec3 LASER_ball_location = LASERbeam.startXYZ;

        // Is the LASER less than 500 units long?
        // (is the last LAZER ball we drew beyond 500 units form the camera?)
        while (glm::distance(::g_pFlyCamera->getEyeLocation(), LASER_ball_location) < maxDistance)
        {
            // Move the next ball 0.1 times the normalized camera direction
            LASER_ball_location += (cameraDirection * 0.10f);
            //DrawDebugSphere(LASER_ball_location, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.05f, program);
        }
        LASERbeam.endXYZ = LASER_ball_location;
    }

    float shortestDistance = maxDistance;

    //Draw the end of this LASER beam
    //DrawDebugSphere(LASERbeam.endXYZ, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, program);

    // Now draw a different coloured ball wherever we get a collision with a triangle
    std::vector<cPhysics::sCollision_RayTriangleInMesh> vec_RayTriangle_Collisions;
    ::g_pPhysicEngine->rayCast(LASERbeam.startXYZ, LASERbeam.endXYZ, vec_RayTriangle_Collisions, false);

    glm::vec4 triColour = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
    float triangleSize = 0.25f;

    for (std::vector<cPhysics::sCollision_RayTriangleInMesh>::iterator itTriList = vec_RayTriangle_Collisions.begin();
        itTriList != vec_RayTriangle_Collisions.end(); itTriList++)
    {
        for (std::vector<cPhysics::sTriangle>::iterator itTri = itTriList->vecTriangles.begin();
            itTri != itTriList->vecTriangles.end(); itTri++)
        {
            //DrawDebugSphere(itTri->intersectionPoint, triColour, triangleSize, program);
            triColour.r -= 0.1f;
            triColour.g -= 0.1f;
            triColour.b += 0.2f;
            triangleSize *= 1.25f;
            distance = glm::distance(itTri->intersectionPoint, ::g_pFlyCamera->getEyeLocation());
            if (distance < shortestDistance)
            {
                shortestDistance = distance;
            }
        }
    }
    return shortestDistance;
}


int main(void)
{
    
    AABBOctTree();




    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();  // Get primary monitor
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);  // Get the monitor's video mode

    window = glfwCreateWindow(mode->width, mode->height, "THE LAST WISH", monitor, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Callback for keyboard, but for "typing"
    // Like it captures the press and release and repeat
    glfwSetKeyCallback(window, key_callback);

    // 
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowFocusCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    // Resets the FBO when we change window size
    // https://www.glfw.org/docs/3.3/window_guide.html#window_events
    glfwSetWindowSizeCallback(window, window_size_callback);



    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);


  //--------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  //ImGui_ImplGlfw_InitForOpenGL(imguiWindow, true);
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
  //--------------------------------------------------------------------------IMGUI--------------------------------------------------------------------  
  

// **********************************************************
//    ____  _               _           
//   / ___|| |__   __ _  __| | ___ _ __ 
//   \___ \| '_ \ / _` |/ _` |/ _ \ '__|
//    ___) | | | | (_| | (_| |  __/ |   
//   |____/|_| |_|\__,_|\__,_|\___|_|   
//                                      

    cShaderManager* pShaderManager = new cShaderManager();

    cShaderManager::cShader vertexShader;
    vertexShader.fileName = "assets/shaders/vertex01.glsl";

    cShaderManager::cShader geometryShader;
//    geometryShader.fileName = "assets/shaders/geom_pass_through.glsl";
    geometryShader.fileName = "assets/shaders/geom_split_triangle.glsl";
//    geometryShader.fileName = "assets/shaders/geom_DrawNormal_lines.glsl";

    cShaderManager::cShader fragmentShader;
    fragmentShader.fileName = "assets/shaders/fragment01.glsl";

    //if ( ! pShaderManager->createProgramFromFile("shader01",
    //                                             vertexShader, fragmentShader))
    if ( ! pShaderManager->createProgramFromFile("shader01",
                                                 vertexShader, geometryShader, fragmentShader))
    {
        std::cout << "Error: " << pShaderManager->getLastError() << std::endl;
    }
    else
    {
        std::cout << "Shader built OK" << std::endl;
    }

    program = pShaderManager->getIDFromFriendlyName("shader01");

    glUseProgram(program);


    
    cShaderManager::cShaderProgram* pShaderProgram
        = pShaderManager->pGetShaderProgramFromFriendlyName("shader01");

    std::string strUniformSummary = pShaderProgram->getActiveUniformSummary();

    //-----------------------------------------------------------------------------ShadowShader----------------------------------------------------------------------------------------
    cShaderManager::cShader shadowVertexShader;
    shadowVertexShader.fileName = "assets/shaders/shadow_depth_vertex.glsl";

    cShaderManager::cShader shadowFragmentShader;
    shadowFragmentShader.fileName = "assets/shaders/shadow_depth_fragment.glsl";

    if (!pShaderManager->createProgramFromFile("shadowShader",
        shadowVertexShader, shadowFragmentShader))
    {
        std::cout << "Error: " << pShaderManager->getLastError() << std::endl;
    }
    else
    {
        std::cout << "Shader built OK" << std::endl;
    }

    shadowShaderProgram = pShaderManager->getIDFromFriendlyName("shadowShader");
    
    pShaderProgram = pShaderManager->pGetShaderProgramFromFriendlyName("shadowShader");
    //-----------------------------------------------------------------------------ShadowShader----------------------------------------------------------------------------------------

    strUniformSummary = pShaderProgram->getActiveUniformSummary();
// **********************************************************

//    std::cout << strUniformSummary << std::endl;

    ::g_pMyLuaMasterBrain = new cLuaBrain();


//    cVAOManager* pMeshManager = new cVAOManager();
    ::g_pMeshManager = new cVAOManager();


    // Traversing the path
    ::g_pTerrainPathChooser = new cTerrainPathChooser(::g_pMeshManager);
    // Set the terrain, etc. 
    // HACK:
    ::g_pTerrainPathChooser->setTerrainMesh(
        "assets/models/Simple_MeshLab_terrain_x5_xyz_N_uv.ply", 
        glm::vec3(0.0f, -175.0f, 0.0f));        // Offset of mesh
    //
    ::g_pTerrainPathChooser->startXYZ = glm::vec3(-500.0f, -75.0f, -500.0f);
    ::g_pTerrainPathChooser->destinationXYZ = glm::vec3(+500.0f, -75.0f, +500.0f);





    ::g_pPhysicEngine = new cPhysics();
    // For triangle meshes, let the physics object "know" about the VAO manager
    ::g_pPhysicEngine->setVAOManager(::g_pMeshManager);


    // Start up the PhysX middleware...
    ::g_pPhysX = new cPhysXWraper();
    ::g_pPhysX->initPhysics(true);


    ::g_pCommandDirector = new cCommandGroup();
    ::g_pCommandFactory = new cCommandFactory();
    // 
    // Tell the command factory about the phsyics and mesh stuff
    ::g_pCommandFactory->setPhysics(::g_pPhysicEngine);
    // (We are passing the address of this...)
    ::g_pCommandFactory->setVectorOfMeshes(&g_vecMeshesToDraw);

    // This also adds physics objects to the phsyics system
    AddModelsToScene(::g_pMeshManager, program);


     
    ::g_pFlyCamera = new cBasicFlyCamera();
    ::g_pFlyCamera->setEyeLocation(glm::vec3(0.0f, -25.0f, -75.0f));

// To see the terrain from high above
//    ::g_pFlyCamera->setEyeLocation(glm::vec3(72.2f, 1270.0f, -1123.0f));
//    ::g_pFlyCamera->pitchUpDown(-45.0f);

    // To see the Galactica:
//    ::g_pFlyCamera->setEyeLocation(glm::vec3(10'000.0f, 25'000.0f, 160'000.0f));
    // Rotate the camera 180 degrees
//    ::g_pFlyCamera->rotateLeftRight_Yaw_NoScaling(glm::radians(180.0f));



    glUseProgram(program);

    // Enable depth buffering (z buffering)
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glEnable.xhtml
    glEnable(GL_DEPTH_TEST);

    cLowPassFilter frameTimeFilter;
//    frameTimeFilter.setNumSamples(30000);

    double currentFrameTime = glfwGetTime();
    double lastFrameTime = glfwGetTime();





    // Set up the lights
    ::g_pLightManager = new cLightManager();
    // Called only once
    ::g_pLightManager->loadUniformLocations(program);

     //Set up one of the lights in the scene
    ::g_pLightManager->theLights[0].position = glm::vec4(25'000.0f, 50'000.0f, -100'000.0f, 1.0f);
    ::g_pLightManager->theLights[0].diffuse = glm::vec4(0.3f, 0.5f, 0.8f, 1.0f);
    ::g_pLightManager->theLights[0].atten.y = 3.0f;
    ::g_pLightManager->theLights[0].atten.z = 0.0004f;

    ::g_pLightManager->theLights[0].param1.x = 0.0f;    // Point light (see shader)
    ::g_pLightManager->theLights[0].param2.x = 1.0f;    // Turn on (see shader)

    //::g_pLightManager->theLights[0].position = glm::vec4(25'000.0f, 50'000.0f, -100'000.0f, 1.0f);
    //::g_pLightManager->theLights[0].diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    //::g_pLightManager->theLights[0].atten.y = 0.000006877f;
    //::g_pLightManager->theLights[0].atten.z = 0.0000000001184f;

    //::g_pLightManager->theLights[0].param1.x = 2.0f;    // Point light (see shader)
    //::g_pLightManager->theLights[0].direction = glm::vec4(0.0f, -0.7f, 0.0f, 1.0f);


    //::g_pLightManager->theLights[0].param2.x = 1.0f;    // Turn on (see shader)


 




    ::g_pTextures = new cBasicTextureManager();

    ::g_pTextures->SetBasePath("assets/textures");

    std::cout << "Loading textures...";

    //::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("dua-lipa-promo.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("Puzzle_parts.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("Non-uniform concrete wall 0512-3-1024x1024.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("UV_Test_750x750.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("shape-element-splattered-texture-stroke_1194-8223.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("Grey_Brick_Wall_Texture.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("dirty-metal-texture_1048-4784.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("bad_bunny_1920x1080_24bit_black_and_white.bmp");
    ////
    //::g_pTextures->Create2DTextureFromBMPFile("SurprisedChildFace.bmp");
    //// 
    //::g_pTextures->Create2DTextureFromBMPFile("Canadian_Flag_Texture.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("Chinese_Flag_Texture.bmp");
    ////
    //::g_pTextures->Create2DTextureFromBMPFile("solid_black.bmp");
    //::g_pTextures->Create2DTextureFromBMPFile("tt_pistol_Albedo.bmp");
    ////
    //::g_pTextures->Create2DTextureFromBMPFile("SpidermanUV_square.bmp");

    ::g_pTextures->Create2DTextureFromBMPFile("Floor.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("door.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Wall.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Ceiling.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Keys.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("heart.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("blood.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("ammo.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("bullet.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("tv.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("objective.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("flash.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("SM_overhead_light_T_overhead_light_2_BaseColor.bmp");

    ::g_pTextures->Create2DTextureFromBMPFile("tt_pistol_Albedo.bmp");


    ::g_pTextures->Create2DTextureFromBMPFile("WallNormal.bmp");

    ::g_pTextures->Create2DTextureFromBMPFile("ammo_NRM.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Ceiling_NRM.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("door_NRM.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Floor_NRM.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("Keys_NRM.bmp");
    ::g_pTextures->Create2DTextureFromBMPFile("SM_overhead_light_T_overhead_light_2_BaseColor_NRM.bmp");

    // Load the space skybox
    std::string errorString;
    ::g_pTextures->SetBasePath("assets/textures/CubeMaps");
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("Space",
        "SpaceBox_right1_posX.bmp", 
        "SpaceBox_left2_negX.bmp",
        "SpaceBox_top3_posY.bmp", 
        "SpaceBox_bottom4_negY.bmp",
        "SpaceBox_front5_posZ.bmp", 
        "SpaceBox_back6_negZ.bmp", true, errorString))
    {
        std::cout << "Loaded space skybox" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space skybox because: " << errorString << std::endl;
    }
        
    //std::cout << "glTexStorage2D():" << glTexStorage2D << std::endl;
    //std::cout << "glCompileShader():" << glCompileShader << std::endl;

    //FARPROC glTexStorage2DProc = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "glTexStorage2D");
    //std::cout << "glCompileShader():" << glTexStorage2DProc << std::endl;
    //FARPROC glCompileShaderProc = GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "glCompileShader");
    //std::cout << "glCompileShader():" << glCompileShaderProc << std::endl;

    // Load the sunny day cube map
    if (::g_pTextures->CreateCubeTextureFromBMPFiles("SunnyDay",
        "TropicalSunnyDayLeft2048.bmp",
        "TropicalSunnyDayRight2048.bmp",
        "TropicalSunnyDayUp2048.bmp",
        "TropicalSunnyDayDown2048.bmp",
        "TropicalSunnyDayFront2048.bmp",
        "TropicalSunnyDayBack2048.bmp",
        true, errorString))
    {
        std::cout << "Loaded space SunnyDay" << std::endl;
    }
    else
    {
        std::cout << "ERROR: Didn't load space SunnyDay because: " << errorString << std::endl;
    }
        
        
    std::cout << "done." << std::endl;
        
        


    //glGet with argument GL_ACTIVE_TEXTURE, or GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS.
    // 
    // void glGetIntegerv(GLenum pname, GLint* data);
    // 
    //GLint iActiveTextureUnits = 0;
    //glGetIntegerv(GL_ACTIVE_TEXTURE, &iActiveTextureUnits);
    //std::cout << "GL_ACTIVE_TEXTURE = " << (iActiveTextureUnits - GL_TEXTURE0) << std::endl;

    GLint iMaxCombinedTextureInmageUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &iMaxCombinedTextureInmageUnits);
    std::cout << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = " << iMaxCombinedTextureInmageUnits << std::endl;

    // data returns one value, the maximum number of components of the inputs read by the fragment shader, 
    // which must be at least 128.
    GLint iMaxFragmentInputComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_INPUT_COMPONENTS, &iMaxFragmentInputComponents);
    std::cout << "GL_MAX_FRAGMENT_INPUT_COMPONENTS = " << iMaxFragmentInputComponents << std::endl;
    

    // data returns one value, the maximum number of individual floating - point, integer, or boolean values 
    // that can be held in uniform variable storage for a fragment shader.The value must be at least 1024. 
    GLint iMaxFragmentUniformComponents = 0;
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &iMaxFragmentUniformComponents);
    std::cout << "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = " << iMaxFragmentUniformComponents << std::endl;
        

    //  Turn on the blend operation
    //glEnable(GL_BLEND);
    // Do alpha channel transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // Is the default (cull back facing polygons)
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // HACK:
    unsigned int numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing = 0;


//    // Camera view from withing the warehouse
//    cFBO_RGB_depth FBO_WarehouseView;
//    std::string FBOError;
//    if (!FBO_WarehouseView.init(1920, 1080, FBOError))
////    if (!FBO_WarehouseView.init(128, 64, FBOError))
////    if (!FBO_WarehouseView.init(3840 * 2, 2180 * 2, FBOError))
//    {
//        std::cout << "Error: FBO.init(): " << FBOError << std::endl;
//    }
//    else
//    {
//        std::cout << "FBO created OK" << std::endl;
//    }

    // Deferred rendering Geometry "G" buffer
    int screen_width, screen_height;
    glfwGetFramebufferSize(window, &screen_width, &screen_height);

    ::g_pFBO_G_Buffer = new cFBO_deferred();
    std::string FBOinitError;
    if ( ! ::g_pFBO_G_Buffer->init(screen_width, screen_height, FBOinitError) )
    {
        std::cout << "ERROR: Can't init deferred FBO buffer because: "
            << FBOinitError << std::endl;
    }
    else
    {
        std::cout << "Deferred FBO init() OK." << std::endl;
    }
 

//    ::g_pParticles = new cParticleEmitter();
    ::g_pParticles = new cParticleEmitter_2();
    ::g_pParticles->SetMaximumNumberOfParticles(15'000);

    ::g_pParticles->SetSourceLocation(glm::vec3(25.0f, -20.0f, 0.0f));
    ::g_pParticles->SetInitalVelocity(
        glm::vec3(-1.0f, 3.0f, -1.0f),        // Min
        glm::vec3( 1.0f, 10.0f, 1.0f));       // Max

    
    //glUseProgram(shadowShaderProgram);
    //g_pShadowCubeMapFBO = new cDepthCubeMap();
    //if (!::g_pShadowCubeMapFBO->init(screen_width, screen_height, FBOinitError))
    //{
    //    std::cout << "ERROR: Can't init depth FBO buffer because: "
    //        << FBOinitError << std::endl;
    //}
    //else
    //{
    //    std::cout << "Depth FBO init() OK." << std::endl;
    //}

    glUseProgram(program);

    // --------------------------------------------------------------------------AudioStuff-----------------------------------------------------------------------------------------------

    g_pSoundManager = new cSoundManager();
    g_pSoundManager->Init();

    //Gun Sounds
    g_pSoundManager->LoadSound("PistolFire", "assets/Audio/PistolFire.wav");
    g_pSoundManager->LoadSound("GunJam", "assets/Audio/GunJam.wav");

    //Player Sounds
    g_pSoundManager->LoadSound("PlayerHit", "assets/Audio/PlayerHit.ogg");
    g_pSoundManager->LoadSound("PlayerDeath", "assets/Audio/PlayerDeath.wav");

    //Misc
    g_pSoundManager->LoadSound("LowHpLoop", "assets/Audio/LowHpLoop.wav", false, true);
    g_pSoundManager->LoadSound("BgMusic", "assets/Audio/BgMusic.mp3", false, true);


    //Zombie Sounds
    g_pSoundManager->LoadSound("ZombieScream", "assets/Audio/ZombieScream.wav", true);
    g_pSoundManager->LoadSound("ZombieDeath", "assets/Audio/ZombieDeath.wav", true);

    //Pickups Sounds
    g_pSoundManager->LoadSound("AmmoPickup", "assets/Audio/AmmoPickup.wav");
    g_pSoundManager->LoadSound("KeyPickup", "assets/Audio/KeyPickup.wav");
    g_pSoundManager->LoadSound("HealthPickup", "assets/Audio/ZombieScream.wav");

    g_pSoundManager->LoadSound("MetalDoorOpen", "assets/Audio/MetalDoorOpen.wav");



    g_pSoundManager->PlaySoundEffect("BgMusic");

    // --------------------------------------------------------------------------AudioStuff-----------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------PlayerStuff----------------------------------------------------------------------------------------------


    pTimelineManager = new cTimelineManager();

    Model* LeonardModel = new Model("res/Leonard/Leonard.dae");
    LeonardModel->positionXYZ = glm::vec3(35.0f, 1.0f, 5.0f);
    LeonardModel->uniformScale = 1.0f;
    LeonardModel->rotationEulerXYZ = glm::vec3(0.0f, 180.0f, 0.0f);
    

    Animation* idle = new Animation("res/Animations/Idle.dae", LeonardModel);
    Animation* walkForward = new Animation("res/Animations/WalkForward.dae", LeonardModel);
    Animation* aim = new Animation("res/Animations/Pistol Idle.dae", LeonardModel);
    Animation* death = new Animation("res/Animations/playerDeath.dae", LeonardModel);
    Animation* hit = new Animation("res/Animations/Hit1.dae", LeonardModel);
    Animation* pistolWalk = new Animation("res/Animations/PistolWalk.dae", LeonardModel);

    g_pAnimator = new Animator(idle);
    //g_pAnimator->AddAnimation("Idle", idle);
    //g_pAnimator->AddAnimation("Walk", walkForward);
    //g_pAnimator->AddAnimation("Aim", aim);

    sModelAnimatorLink* LeonardLink = new sModelAnimatorLink();
    LeonardLink->model = LeonardModel;
    LeonardLink->animator = g_pAnimator;

    vecModelAnimatorLinks.push_back(LeonardLink);

    sAnimationNames* playerAnimIdle = new sAnimationNames();
    playerAnimIdle->name = "Idle";
    playerAnimIdle->animation = idle;

    sAnimationNames* playerAnimWalk = new sAnimationNames();
    playerAnimWalk->name = "Walk";
    playerAnimWalk->animation = walkForward;

    sAnimationNames* playerAnimAim = new sAnimationNames();
    playerAnimAim->name = "Aim";
    playerAnimAim->animation = aim;

    sAnimationNames* playerAnimDeath = new sAnimationNames();
    playerAnimDeath->name = "Die";
    playerAnimDeath->animation = death;

    sAnimationNames* playerAnimHit = new sAnimationNames();
    playerAnimHit->name = "Hit";
    playerAnimHit->animation = hit;

    sAnimationNames* playerPistolWalkAnim = new sAnimationNames();
    playerPistolWalkAnim->name = "PistolWalk";
    playerPistolWalkAnim->animation = pistolWalk;

    pPlayerReference = new cPlayer();

    pPlayerReference->vecPlayerAnims.push_back(playerAnimIdle);
    pPlayerReference->vecPlayerAnims.push_back(playerAnimWalk);
    pPlayerReference->vecPlayerAnims.push_back(playerAnimAim);
    pPlayerReference->vecPlayerAnims.push_back(playerAnimDeath);
    //pPlayerReference->vecPlayerAnims.push_back(playerAnimHit);
    pPlayerReference->vecPlayerAnims.push_back(playerPistolWalkAnim);

    pPlayerReference->PlayerBeginPlay(LeonardModel, g_pAnimator, program);


    //// Set up one of the lights in the scene
    ::g_pLightManager->theLights[1].position = glm::vec4(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, 2.5f, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z, 1.0f);
    ::g_pLightManager->theLights[1].diffuse = glm::vec4(0.5f, 1.0f, 1.0f, 1.0f);
    ::g_pLightManager->theLights[1].atten.y = 0.5f;
    ::g_pLightManager->theLights[1].atten.z = 0.001f;

    ::g_pLightManager->theLights[1].param1.x = 1.0f;    // Spot light (see shader)
    ::g_pLightManager->theLights[1].direction = glm::vec4(pPlayerReference->getPlayerForward(), 1.0f);
    ::g_pLightManager->theLights[1].param1.y = 1.0f;   //  y = inner angle
    ::g_pLightManager->theLights[1].param1.z = 40.0f;  //  z = outer angle

    ::g_pLightManager->theLights[1].param2.x = 1.0f;    // Turn on (see shader)

    pLevelGenerator = new cLevelGenerator();

    //pLevelGenerator->SetupMap(50, 50);
    pLevelGenerator->SetupMap(50, 18);
    pLevelGenerator->ReadMap("assets/Misc/Map.txt");
    pLevelGenerator->GenerateMap();


    //---------------------------------------------------------------------------PlayerStuff----------------------------------------------------------------------------------------------
    int i = 0;
    for (sMesh* pCurDoor : g_vecDoors) 
    {
        cDoor* pInteractableDoor = new cDoor(pCurDoor);
        //createStaticWall(PxVec3(pCurDoor->positionXYZ.x, 5.0f, pCurDoor->positionXYZ.z), PxVec3(1.0f, 1.5f, 0.5f), pCurDoor->rotationEulerXYZ);

        pCurDoor = g_pFindMeshByFriendlyName("Door" + std::to_string(i));
        pInteractableDoor->InteractableBeginPlay(pCurDoor);
        g_pVecInteractableDoors.push_back(pInteractableDoor);
        i++;
    }


    {
        cKey* pRedKey = new cKey();
        sMesh* pRedKeyMesh = g_pFindMeshByFriendlyName("RedKey");
        pRedKey->InteractableBeginPlay(pRedKeyMesh);
        pRedKey->pInteractableTrigger->uniformScale = 2.0f;
        g_pVecInteractableKeys.push_back(pRedKey);
    }
    {
        cKey* pYellowKey = new cKey();
        sMesh* pYellowKeyMesh = g_pFindMeshByFriendlyName("YellowKey");
        pYellowKey->InteractableBeginPlay(pYellowKeyMesh);
        pYellowKey->pInteractableTrigger->uniformScale = 2.0f;
        g_pVecInteractableKeys.push_back(pYellowKey);
    }
    { 
        cKey* pBlueKey = new cKey();
        sMesh* pBlueKeyMesh = g_pFindMeshByFriendlyName("BlueKey");
        pBlueKey->InteractableBeginPlay(pBlueKeyMesh);
        pBlueKey->pInteractableTrigger->uniformScale = 2.0f;
        g_pVecInteractableKeys.push_back(pBlueKey);
    }
    {
        cKey* pGreenKey = new cKey();
        sMesh* pGreenKeyMesh = g_pFindMeshByFriendlyName("GreenKey");
        pGreenKey->InteractableBeginPlay(pGreenKeyMesh);
        pGreenKey->pInteractableTrigger->uniformScale = 2.0f;
        g_pVecInteractableKeys.push_back(pGreenKey);
    }


    //g_myParticleEmmiter = new cParticleEmmiterOwn();
    //g_myParticleEmmiter->InitializeParticles();


    double lastX = 0.0f; // Previous mouse x position
    double lastY = 0.0f; // Previous mouse y position

    bool isMouseVisible = false;

    cFBO_RGB_depth objectiveFBO;
    FBOInit(objectiveFBO);

    float objectiveBlendTime = 0.0f;
    bool bIsIPressed = false;
    bool bWasIKeyPressed = false; // To track previous state


    cFBO_RGB_depth FBOCam1;
    FBOInit(FBOCam1);
    glm::vec3 cam1 = glm::vec3(0.0f, 40.0f, 0.0f);

    InitDebugLineVAO(shadowShaderProgram);
    pPlayerReference->UpdateSpringArm();

    bool bIsThreadedDoOnce = false;

    float iTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        std::stringstream ssTitle;

        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        ratio = width / (float)height;

        DrawDebugSphere(LeonardModel->positionXYZ, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 10.0f, program);
        //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        DrawZombieShooterHUD(deltaTime);
        if (bIsAiming) DrawCrosshair(width, height);
        ShowControlsUIBottomRight();
        RenderMinimap(FBOCam1.colourTexture_0_ID, width, height);
        //RenderImGui();
        
        //ImGui::Render();
        //ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------

        // Calculate elapsed time
        // We'll enhance this
        currentFrameTime = glfwGetTime();
        double tempDeltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Set a limit on the maximum frame time
        const double MAX_FRAME_TIME = 1.0 / 60.0;   // 60Hz (16 ms)
        if (tempDeltaTime > MAX_FRAME_TIME)
        {
            tempDeltaTime = MAX_FRAME_TIME;
        }

        // Add this sample to the low pass filer ("averager")
        frameTimeFilter.addSample(tempDeltaTime);
        // 
        deltaTime = frameTimeFilter.getAverage();
        
        glm::mat4 matProjection = glm::mat4(1.0f);
        glm::mat4 matView = glm::mat4(1.0f);
        glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

        //--------------------------------------------------------------------------PlayerCam----------------------------------------------------------------------------------------
        if (!bIsAiming) 
        {
            matView = glm::lookAt(
                ::g_pFlyCamera->getEyeLocation(),
                glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y + 1.0f, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z),
                upVector);
        }
        else 
        {
            matView = glm::lookAt(
                ::g_pFlyCamera->getEyeLocation(),
                glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y + 1.0f + pPlayerReference->getForwardVector().y, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z) +
                glm::vec3(pPlayerReference->getPlayerRight().x * 0.4f, 0.5f, pPlayerReference->getPlayerRight().z * 0.4f),
                upVector);
        }
        if (pPlayerReference->Health <= 0.0f) 
        {
            matView = glm::lookAt(
                ::g_pFlyCamera->getEyeLocation(),
                glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ),
                upVector);
        }
        //--------------------------------------------------------------------------PlayerCam----------------------------------------------------------------------------------------
        matProjection = glm::perspective(FoV,
            ratio,
            0.1f,
            5000.0f);

        // Physic update and test 
        // (The phsyics "engine" is NOT updating the Verlet (soft bodies) 
        //  in this method...)
        ::g_pPhysicEngine->StepTick(deltaTime);

        
        
// THREADED NOW
//         
//  Update this from THE LAST FRAME
// 
        // Update the meshes in the VAO to match any soft bodies
        // MUST keep this on the OpenGL thread that "has context"
        // (i.e. the one that we created the GL window on)
        // (i.e. we are NOT going to move this to another thread)
        ::g_pPhysicEngine->updateSoftBodyMeshes(program);

        //----------------------------------------------------------------AudioStuff--------------------------------------------------------------------

        FMOD_VECTOR playerPos;
        playerPos.x = pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x;
        playerPos.y = pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y;
        playerPos.z = pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z;

        FMOD_VECTOR forwardVector;
        forwardVector.x = pPlayerReference->getForwardVector().x;
        forwardVector.y = pPlayerReference->getForwardVector().y;
        forwardVector.z = pPlayerReference->getForwardVector().z;

        FMOD_VECTOR upVec = { 0.0f, 1.0f, 0.0f };

        g_pSoundManager->SetListenerPosition(playerPos, forwardVector, upVec);
        g_pSoundManager->Update();

        //----------------------------------------------------------------AudioStuff--------------------------------------------------------------------


        //---------------------------------------------------------------------Player Stuff------------------------------------------------------------------------------
        LineTraceForward(program, 500.0f);

        frontDistance = LineTrace(::g_pFlyCamera->getTargetRelativeToCamera(), program, 100.0f);
        rightDistance = LineTrace(::g_pFlyCamera->getRightVector() * (-1.0f), program, 100.0f);
        leftDistance = LineTrace(::g_pFlyCamera->getRightVector(), program, 100.0f);
        backDistance = LineTrace(::g_pFlyCamera->getTargetRelativeToCamera() * (-1.0f), program, 150.0f);
        downDistance = LineTrace(glm::vec3(0.0f, -1.0f, 0.0f), program, 150.0f);


        //if (frontDistance > collisionThreshold)
        //    ::g_pFlyCamera->moveForward(CAMERA_MOVE_SPEED);

        //if (backDistance > collisionThreshold)
        //    ::g_pFlyCamera->moveForward(-CAMERA_MOVE_SPEED);

        //if (leftDistance > collisionThreshold)
        //    ::g_pFlyCamera->moveLeftRight(CAMERA_MOVE_SPEED);

        //if (rightDistance > collisionThreshold)
        //    ::g_pFlyCamera->moveLeftRight(-CAMERA_MOVE_SPEED);


        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Calculate the change in mouse position (delta)
        deltaX = (xpos - lastX);
        deltaY = (ypos - lastY);

        pPlayerReference->UpdateCameraRotation(-deltaX, deltaY);
        pPlayerReference->UpdateSpringArm();



        ::g_pPhysicEngine->StepTick(deltaTime);

        lastX = xpos;
        lastY = ypos;

        pPlayerReference->PlayerUpdate(deltaTime);


        

        //if (downDistance <= 150.0f) {
        //    g_pFlyCamera->setEyeLocation(g_pFlyCamera->getEyeLocation().x, 3.0f, g_pFlyCamera->getEyeLocation().z);
        //}

        //pPlayerReference->DestroyExpiredBullets();
        //pPlayerReference->CheckBulletCollisions();

        for (cDoor* pCurInteractableDoor : g_pVecInteractableDoors) {
            DrawInteractUI(pCurInteractableDoor->doorMesh->positionXYZ, pPlayerReference->pPlayerSkeletalMesh->positionXYZ, matView, matProjection, width, height);
            pCurInteractableDoor->InteractableUpdate(deltaTime);
        }
        for (cKey* pCurInteractableKey : g_pVecInteractableKeys) {
            if(pCurInteractableKey) pCurInteractableKey->InteractableUpdate(deltaTime);
        }
        for (cHealth* pCurInteractableHealth : g_pVecInteractableHealth) {
            if (pCurInteractableHealth) pCurInteractableHealth->InteractableUpdate(deltaTime);
        }
        for (cAmmo* pCurInteractableAmmo : g_pVecInteractableAmmo) {
            if (pCurInteractableAmmo) pCurInteractableAmmo->InteractableUpdate(deltaTime);
        }

        pTimelineManager->UpdateAll(deltaTime);

        pLevelGenerator->RotateKeys(deltaTime);

        for(int i = 0; i < pLevelGenerator->numHeart; i++){
            sMesh* Health = g_pFindMeshByFriendlyName("Heart" + std::to_string(i));
            if (Health) Health->rotationEulerXYZ.y += 120.0f * deltaTime;
        }
        for (int j = 0; j < pLevelGenerator->numAmmo; j++) {
            sMesh* Ammo = g_pFindMeshByFriendlyName("Ammo" + std::to_string(j));
            if (Ammo) Ammo->rotationEulerXYZ.y += 120.0f * deltaTime;
        }

        for (cMasterZombie* pZombie : AllZombies) {
            pZombie->ZombieUpdate(deltaTime);
            //pZombie->zombieDeltaTime = deltaTime;
            //pZombie->ZombieUpdateRender(); // Only rendering-safe stuff here
        }

        for (cBullet* pCurBullet : vecBullets) {
            pCurBullet->Update(deltaTime);
        }
        //AllZombies[0]->ZombieUpdate(deltaTime);

        //---------------------------------------------------------------------Player Stuff------------------------------------------------------------------------------



                // Inside your render loop:
        //if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
        //{
        //    // Toggle the mouse visibility
        //    //isMouseVisible = false;
        //    isMouseVisible = !isMouseVisible;

        //    if (isMouseVisible)
        //    {
        //        // Show the mouse cursor
        //        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        //    }
        //    else {
        //        // Hide the mouse cursor and lock it to the window
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        //    }
        //    // Small delay to prevent multiple toggles in one frame
        //    //glfwPollEvents();
        //}


        //if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        //{
        //    bIsIPressed = true;
        //    glUniform1f(glGetUniformLocation(program, "bIsIPressed"), (GLfloat)GL_TRUE);  // True
        //    //objectiveBlendTime += deltaTime;
        //}
        //if (objectiveBlendTime >= 1.0f) {
        //    bIsIPressed = false;
        //    glUniform1f(glGetUniformLocation(program, "bIsIPressed"), (GLfloat)GL_FALSE);  // True
        //    objectiveBlendTime = 0.0f;
        //}
        //if (bIsIPressed) {
        //    objectiveBlendTime += deltaTime;
        //}


        //****************************************************Objectives************************************************************************************
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
            if (!bWasIKeyPressed) { // Only toggle when the key was not pressed last frame
                bIsIPressed = !bIsIPressed; // Toggle the value
                glUniform1f(glGetUniformLocation(program, "bIsIPressed"), bIsIPressed ? (GLfloat)GL_TRUE : (GLfloat)GL_FALSE);
            }
            bWasIKeyPressed = true; // Mark key as pressed
            objectiveBlendTime = 0.3f;
        }
        else {
            bWasIKeyPressed = false; // Reset when key is released
            objectiveBlendTime += deltaTime;
            if (objectiveBlendTime >= 1.0f) {
                objectiveBlendTime = 1.0f;
            }
        }
        //****************************************************Objectives************************************************************************************
        
        //****************************************************Threading************************************************************************************
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            bUseThreading = !bUseThreading;
        }
        //bUseThreading = false;

        //****************************************************Threading************************************************************************************

        //std::cout << "Entering particle section" << std::endl;
        //***************************************************ParticleUpdate**********************************************
        //if (g_myParticleEmmiter->particles.size() > 0) {
        //    g_myParticleEmmiter->UpdateParticles(deltaTime);
        //    g_myParticleEmmiter->DrawParticles();
        //}
        //***************************************************ParticleUpdate**********************************************

        //*****************************************************FBO_TOP_DOWN********************************************************
        sMesh* pMinimap = g_pFindMeshByFriendlyName("MiniMap");
        pMinimap->positionXYZ = glm::vec3(25.0f, 2.0f, 5.0f);
        pMinimap->rotationEulerXYZ = glm::vec3(0.0f, 90.0f, 0.0f);
        pMinimap->bIsFBO = true;
        pMinimap->bIsMinimap = true;
        //glm::vec3 screenPos(width - 100.0f, height - 100.0f, 0.1f); // Z = near plane (0.0f to 1.0f)
        //glm::vec3 minimapWorldPos = ScreenToWorld(screenPos, matView, matProjection, width, height);

        //pMinimap->positionXYZ = minimapWorldPos;

        //glm::vec3 camForward = glm::normalize(glm::vec3(matView[0][2], matView[1][2], matView[2][2]));
        //glm::vec3 camRight = glm::normalize(glm::vec3(matView[0][0], matView[1][0], matView[2][0]));
        //glm::vec3 camUp = glm::normalize(glm::vec3(matView[0][1], matView[1][1], matView[2][1]));

        //// Create rotation matrix from camera basis
        //glm::mat4 billboardMatrix(1.0f);
        //billboardMatrix[0] = glm::vec4(camRight, 0.0f);
        //billboardMatrix[1] = glm::vec4(camUp, 0.0f);
        //billboardMatrix[2] = glm::vec4(camForward * -1.0f, 0.0f);  // negative Z forward
        //billboardMatrix[3] = glm::vec4(0.0f); // position is handled elsewhere

        //// Convert to Euler (if your mesh uses Euler)
        //glm::vec3 eulerAngles = glm::eulerAngles(glm::quat_cast(billboardMatrix));
        //pMinimap->rotationEulerXYZ = eulerAngles;
        FBOBind(FBOCam1, ratio, program, cam1, glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z), g_orthoSize);
        pMinimap->bIsVisible = false;
        //*****************************************************FBO_TOP_DOWN********************************************************



        // Update the mesh information from the LAST frame:
        // (i.e. from the thread we kicked off last frame)

        // Move the flag to where the viper is
        if (::g_pViperFlagConnector)
        {
            ::g_pViperFlagConnector->UpdateFlagLocation();
        }


        // This might be better inside the StepTick(),
        //  but I'm leaving it here for clarification
        //  or if you DON'T want any soft bodies
        ::g_pPhysicEngine->updateSoftBodies(deltaTime);


        // Update PhysX...
        ::g_pPhysX->update();

        //if( g_pParticles->GetNumberOfLiveParticles() > 0) g_pParticles->StartParticleUpdateThread();
        ::g_pParticles->Update(deltaTime);

        // Update the commands, too
        ::g_pCommandDirector->Update(deltaTime);


        // Handle any collisions
        if (::g_pPhysicEngine->vec_SphereAABB_Collisions.size() > 0)
        {
            // Yes, there were collisions

            for (unsigned int index = 0; index != ::g_pPhysicEngine->vec_SphereAABB_Collisions.size(); index++)
            {
                cPhysics::sCollision_SphereAABB thisCollisionEvent = ::g_pPhysicEngine->vec_SphereAABB_Collisions[index];

                if (thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y < 0.0f)
                {
                    // Yes, it's heading down
                    // So reverse the direction of velocity
                    thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y = fabs(thisCollisionEvent.pTheSphere->pPhysicInfo->velocity.y);
                }

            }//for (unsigned int index

        }//if (::g_pPhysicEngine->vec_SphereAABB_Collisions




        // **************************************************
        // 
        // RENDER from the inside of the warehouse


//        // Point output to the off-screen FBO
//        glBindFramebuffer(GL_FRAMEBUFFER, FBO_WarehouseView.ID);
//
//        // These will ONLY work on the default framebuffer
////        glViewport(0, 0, width, height);
////       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
////        FBO_WarehouseView.clearColourBuffer(0);
//        FBO_WarehouseView.clearBuffers(true, true);
//
//
//        matProjection = glm::perspective(0.6f,
//            ratio,
//            10.0f,
//            100'000.0f);
//
//        glm::vec3 eyeInsideWarehouse = glm::vec3(-197.0f, 14.0f, -72.0f);
//        float xOffset = 10.0f * glm::sin((float)glfwGetTime() / 2.0f);
//        glm::vec3 atInsideWareHouse =
//            eyeInsideWarehouse + glm::vec3(xOffset, 0.0f, 10.0f);
//        
//        matView = glm::lookAt(eyeInsideWarehouse, atInsideWareHouse, glm::vec3(0.0f, 1.0f, 0.0f));
//
//        RenderScene(program, matProjection, matView, ratio, eyeInsideWarehouse);
//        // 
//        // **************************************************

        // Pass #0 --> Render the G buffer
        //          output to the FBO deferred object
        // 
        // (Pass #1 --> optional effect pass, "2nd pass effect")
        //
        // Pass #2 --> Render the Deferred lighting pass 
        //          output to the screen onto a full screen quad
        //

        // uniform int renderPassNumber;
        GLint renderPassNumber_UL = glGetUniformLocation(program, "renderPassNumber");
        // Pass "0" (regular forward rendering)
        // 


// **************************************************
//     ____           ____         __  __                                 
//    / ___|         | __ ) _   _ / _|/ _| ___ _ __   _ __   __ _ ___ ___ 
//   | |  _   _____  |  _ \| | | | |_| |_ / _ \ '__| | '_ \ / _` / __/ __|
//   | |_| | |_____| | |_) | |_| |  _|  _|  __/ |    | |_) | (_| \__ \__ \
//    \____|         |____/ \__,_|_| |_|  \___|_|    | .__/ \__,_|___/___/
//                                                   |_|                  

        // 
        // Pass "1" for deferred G buffer pass
        glUniform1i(renderPassNumber_UL, 1);



        // Point the output to the G buffer...
        glBindFramebuffer(GL_FRAMEBUFFER, ::g_pFBO_G_Buffer->ID);

        

        // Clear the buffers on the FBO
        // (remember that glClear() only works on the regular screen buffer)
        ::g_pFBO_G_Buffer->clearBuffers(true, true);


        //matView = glm::lookAt(
        //    ::g_pFlyCamera->getEyeLocation(),
        //    ::g_pFlyCamera->getTargetLocation(),
        //    upVector);

        ////--------------------------------------------------------------------------PlayerCam----------------------------------------------------------------------------------------
        //if (!bIsAiming) {
        //    matView = glm::lookAt(
        //        ::g_pFlyCamera->getEyeLocation(),
        //        glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y + 1.0f, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z),
        //        upVector);
        //}
        //else {
        //    matView = glm::lookAt(
        //        ::g_pFlyCamera->getEyeLocation() /*+ pPlayerReference->getPlayerRight() * 0.5f - pPlayerReference->getPlayerForward() * pPlayerReference->SpringArmLength*/,
        //        glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.y + 1.0f + pPlayerReference->getForwardVector().y, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z) +
        //        glm::vec3(pPlayerReference->getPlayerRight().x * 0.4f, 0.5f, pPlayerReference->getPlayerRight().z * 0.4f),
        //        upVector);
        //}
        //if (pPlayerReference->Health <= 0.0f) {
        //    matView = glm::lookAt(
        //        ::g_pFlyCamera->getEyeLocation(),
        //        glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ),
        //        upVector);
        //}
        ////--------------------------------------------------------------------------PlayerCam----------------------------------------------------------------------------------------

        GLint eyeLocation_UL = glGetUniformLocation(program, "eyeLocation");
        glUniform4f(eyeLocation_UL, 
            ::g_pFlyCamera->getEyeLocation().x,
            ::g_pFlyCamera->getEyeLocation().y, 
            ::g_pFlyCamera->getEyeLocation().z, 1.0f);

        //matProjection = glm::perspective(FoV,
        //    ratio,
        //    1.0f,
        //    50'000.0f);



//        // Render the offscreen FBO texture onto where Dua Lipa was...
//        sMesh* pFBOTextureMesh = ::g_pFindMeshByFriendlyName("WareHouseView");
//        // 
//        // Now apply that off-screen texture (from the FBO) to the canadian flag model
////        sMesh* pFBOTextureMesh = ::g_pFindMeshByFriendlyName("Canadian_Flag");
//
//        if (pFBOTextureMesh)
//        {
//            pFBOTextureMesh->bIsVisible = true;
//
//            GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
//            glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);
//
//            GLint matView_UL = glGetUniformLocation(program, "matView");
//            glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);
//
//
//
//            // Connect texture unit #0 to the offscreen FBO
//            glActiveTexture(GL_TEXTURE0);
//
//            // The colour texture inside the FBO is just a regular colour texture.
//            // There's nothing special about it.
//            glBindTexture(GL_TEXTURE_2D, FBO_WarehouseView.colourTexture_0_ID);
////            glBindTexture(GL_TEXTURE_2D, ::g_pTextures->getTextureIDFromName("dua-lipa-promo.bmp"));
//
//            GLint texture01_UL = glGetUniformLocation(program, "texture00");
//            glUniform1i(texture01_UL, 0);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here
//
//            GLint texRatio_0_to_3_UL = glGetUniformLocation(program, "texRatio_0_to_3");
//            glUniform4f(texRatio_0_to_3_UL,
//                1.0f,
//                0.0f,
//                0.0f,
//                0.0f);
//
//            // This is for the blurring effect
//            GLint b_Is_FBO_Texture_UL = glGetUniformLocation(program, "b_Is_FBO_Texture");
//            GLint bUseTextureAsColour_UL = glGetUniformLocation(program, "bUseTextureAsColour");
//
//            glUniform1f(b_Is_FBO_Texture_UL, (float)GL_TRUE);
//            glUniform1f(bUseTextureAsColour_UL, (float)GL_FALSE);
//
//            DrawMesh(pFBOTextureMesh, program, false);
//
//            glUniform1f(b_Is_FBO_Texture_UL, (float)GL_FALSE);
//            glUniform1f(bUseTextureAsColour_UL, (float)GL_TRUE);
//
//            pFBOTextureMesh->bIsVisible = false;
//        }

        //glUniform4f(eyeLocation_UL,
        //    ::g_pFlyCamera->getEyeLocation().x,
        //    ::g_pFlyCamera->getEyeLocation().y,
        //    ::g_pFlyCamera->getEyeLocation().z, 1.0f);

        
        //glUseProgram(shadowShaderProgram);
        /*for (cLightManager::sLight curLight : g_pLightManager->theLights) {
            g_pShadowCubeMapFBO->renderDepthFromLightPers(shadowShaderProgram, curLight.position);
        }

        glActiveTexture(GL_TEXTURE0 + 21);
        glBindTexture(GL_TEXTURE_CUBE_MAP, g_pShadowCubeMapFBO->depthCubemapTex_ID);

        GLint shadowMap_UL = glGetUniformLocation(program, "shadowMap");
        glUniform1i(shadowMap_UL, 21);

        for (cLightManager::sLight curLight : g_pLightManager->theLights) {
            glUniform3f(glGetUniformLocation(shadowShaderProgram, "lightPos"), curLight.position.x, curLight.position.y, curLight.position.z);
            glUniform1f(glGetUniformLocation(shadowShaderProgram, "far_plane"), 1000.0f);
        }*/

        ////--------------------------------------------------------------------------------Animations----------------------------------------------------------------------
        //ourModel.positionXYZ.x += 5.5f * deltaTime;
        //ourModel.uniformScale = 5.0f;
        //RenderAllAnimated(program, matProjection, matView, deltaTime);
        //copZombieModel.RenderAnimatedStuff(program, matProjection, matView, deltaTime);
        //copZombieModel.Draw(program);
        //pLeonardAnimator->UpdateAnimation(deltaTime);
        //pLeonardModel.RenderAnimatedStuff(program, matProjection, matView, deltaTime);

        ////--------------------------------------------------------------------------------Animations----------------------------------------------------------------------

 
        //RenderFBOTextureSingleMesh(pMinimap, FBOCam1, program, matProjection, matView, ratio);
        RenderScene(program, matProjection, matView, ratio, ::g_pFlyCamera->getEyeLocation());
        
       

        GLint b_Is_FBO_Texture_UL = glGetUniformLocation(program, "b_Is_FBO_Texture");
        glUniform1f(b_Is_FBO_Texture_UL, (GLfloat)GL_FALSE);

        //RenderSceneDepth(shadowShaderProgram);



        //glUseProgram(program);

        

// **************************************************


// **************************************************
//    ____        __                        _   _ _       _     _   _                                   
//   |  _ \  ___ / _| ___ _ __ _ __ ___  __| | | (_) __ _| |__ | |_(_)_ __   __ _   _ __   __ _ ___ ___ 
//   | | | |/ _ \ |_ / _ \ '__| '__/ _ \/ _` | | | |/ _` | '_ \| __| | '_ \ / _` | | '_ \ / _` / __/ __|
//   | |_| |  __/  _|  __/ |  | | |  __/ (_| | | | | (_| | | | | |_| | | | | (_| | | |_) | (_| \__ \__ \
//   |____/ \___|_|  \___|_|  |_|  \___|\__,_| |_|_|\__, |_| |_|\__|_|_| |_|\__, | | .__/ \__,_|___/___/
//                                                  |___/                   |___/  |_|                  

// Point the output to the regular framebuffer (the screen)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // These will ONLY work on the default framebuffer
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);


        //+****************************************************DebugLine***************************************************


        
        //if (bIsAiming) {

        //    glm::vec3 crosshairWorldPos = GetCrosshairWorldPos(width, height, matView, matProjection, AllZombies, AllZombies.size());

        //    DrawDebugLineModern(
        //        glm::vec3(pPlayerReference->pPlayerSkeletalMesh->positionXYZ) + getCameraLookDirection() * 1.5f,
        //        crosshairWorldPos,
        //        matView,
        //        matProjection,
        //        shadowShaderProgram         // basic unlit shader with uViewProj & uColor
        //    );
        //}


        //+****************************************************DebugLine***************************************************

        glUseProgram(program);

        ::g_pLightManager->theLights[1].position = glm::vec4(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x + pPlayerReference->getPlayerForward().x * 1.5f, 2.5f , pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z + pPlayerReference->getPlayerForward().z * 1.5f, 1.0f);

        //::g_pLightManager->theLights[1].position = glm::vec4(pPlayerReference->pPlayerSkeletalMesh->positionXYZ, 1.0f);
        ::g_pLightManager->theLights[1].direction = glm::vec4(pPlayerReference->getPlayerForward(), 1.0f);

        ::g_pLightManager->theLights[0].position = glm::vec4(pPlayerReference->pPlayerSkeletalMesh->positionXYZ.x, 3.5f, pPlayerReference->pPlayerSkeletalMesh->positionXYZ.z, 1.0f);

        //::g_pLightManager->theLights[0].diffuse = glm::vec4(0.3, 0.35, 0.4, 1.0f);

        // Pass "3" for deferred lighting pass
        glUniform1i(renderPassNumber_UL, 3);


        // We can render any object
//        sMesh* pFSQ = g_pFindMeshByFriendlyName("New_Viper_Player");
        sMesh* pFSQ = g_pFindMeshByFriendlyName("Full_Screen_Quad");


        // We are setting the camera (view) and projection matrix 
        //  specifically for this shot of the FSQ

        // In our case, the quad is 2x2 in size, centred at the origin
        //  facing along the +ve z axis.
        // It goes from -1.0 to 1.0 on the x and y axes

       pFSQ->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
       pFSQ->bIsVisible = true;
       //pFSQ->rotationEulerXYZ.y += 0.1f;
        // 
        // 
        // Set the camera 
        //
        // ...We could make the quad bigger or move closer
        //
        // The key is we want to the full screen quad to be "too" big,
        //  like it's completely filling the creen and going off the edges
        // (that way whatever resolution or window size, we'll be OK)
        //
        matView = glm::lookAt(
            glm::vec3(0.0f, 0.0f, +1.0f),  // +1 units along the z
            glm::vec3(0.0f, 0.0f, 0.0f),    // Looking at the origin 
            glm::vec3(0.0f, 1.0f, 0.0f));   // "up" is +ve Y


        GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
        GLint matView_UL = glGetUniformLocation(program, "matView");

        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);

        // glm::ortho(
        
        // Watch the near and far plane as we are REALLY close to the quad...
        matProjection = glm::perspective(0.6f,
            ratio,
            0.1f, 2.0f);       // FSQ is 10 units from the camera 
                                // (and it's a flat object)

        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);



        // Connect the FBO deferred textures to the textures in our final salighting pass
        //    uniform sampler2D vertexWorldLocationXYZ_texture;
        //    uniform sampler2D vertexNormalXYZ_texture;
        //    uniform sampler2D vertexDiffuseRGB_texture;
        //    uniform sampler2D vertexSpecularRGA_P_texture;
        //
        // Note here I'm picking texture unit numbers for not particular 
        //  reason. i.e. they don't have to be zero, or match the FBO, or 
        //  even in any order. They are independent of all that
        {
            glActiveTexture(GL_TEXTURE0 + 13);
            glBindTexture(GL_TEXTURE_2D, g_pFBO_G_Buffer->vertexWorldLocationXYZ);
            GLint vertexWorldLocationXYZ_texture_UL 
                = glGetUniformLocation(program, "vertexWorldLocationXYZ_texture");
            glUniform1i(vertexWorldLocationXYZ_texture_UL, 13);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here
        }
        {
            glActiveTexture(GL_TEXTURE0 + 14);
            glBindTexture(GL_TEXTURE_2D, g_pFBO_G_Buffer->vertexNormalXYZ);
            GLint vertexNormalXYZ_texture_UL
                = glGetUniformLocation(program, "vertexNormalXYZ_texture");
            glUniform1i(vertexNormalXYZ_texture_UL, 14);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here
        }
        {
            glActiveTexture(GL_TEXTURE0 + 15);
            glBindTexture(GL_TEXTURE_2D, g_pFBO_G_Buffer->vertexDiffuseRGB);
            GLint vertexDiffuseRGB_texture_UL
                = glGetUniformLocation(program, "vertexDiffuseRGB_texture");
            glUniform1i(vertexDiffuseRGB_texture_UL, 15);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here
        }
        {
            glActiveTexture(GL_TEXTURE0 + 16);
            glBindTexture(GL_TEXTURE_2D, g_pFBO_G_Buffer->vertexSpecularRGA_P);
            GLint vertexSpecularRGA_P_texture_UL
                = glGetUniformLocation(program, "vertexSpecularRGA_P_texture");
            glUniform1i(vertexSpecularRGA_P_texture_UL, 16);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here
        }
        {
            glActiveTexture(GL_TEXTURE0 + 17);
            glBindTexture(GL_TEXTURE_2D, ::g_pTextures->getTextureIDFromName("objective.bmp"));
            GLint objectiveTextureLoc
                = glGetUniformLocation(program, "objectiveTexture");
            glUniform1i(objectiveTextureLoc, 17);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here
        }
        //{
        //    glActiveTexture(GL_TEXTURE0 + 18);
        //    glBindTexture(GL_TEXTURE_2D, FBOCam1.colourTexture_0_ID);
        //    GLint FBOTexture_UL
        //        = glGetUniformLocation(program, "FBOTexture");
        //    glUniform1i(FBOTexture_UL, 18);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here
        //}

        glUniform1f(glGetUniformLocation(program, "deltaTime"), objectiveBlendTime);
        
        iTime += deltaTime;
        glUniform1f(glGetUniformLocation(program, "iTime"), iTime);
        glUniform1f(glGetUniformLocation(program, "health"), pPlayerReference->Health);
        // Also pass the current screen size
        // uniform vec2 screenSize_width_height
        int screen_width, height_height;
        glfwGetFramebufferSize(window, &screen_width, &height_height);

        GLint screenSize_width_height_UL
            = glGetUniformLocation(program, "screenSize_width_height");

        glUniform2f(screenSize_width_height_UL, 
                     (GLfloat)screen_width, 
                     (GLfloat)height_height);



        glm::mat4 matModel = glm::mat4(1.0f);   // Identity
        DrawMesh(pFSQ, matModel, program, false);

        // Hide the quad from rendering anywhere else
        //pFSQ->bIsVisible = false;

// **************************************************
        pMinimap->bIsVisible = false;
        
        // Load any outstanding models async...
        ::g_pMeshManager->LoadAsynModels(program);

        // Handle async IO stuff
        //handleKeyboardAsync(window);
        //handleMouseAsync(window);
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            handleKeyboardAsync(window);
        }
        if (!ImGui::GetIO().WantCaptureMouse) {
            handleMouseAsync(window);
        }



        ////  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
        //ImGui_ImplOpenGL3_NewFrame();
        //ImGui_ImplGlfw_NewFrame();
        //ImGui::NewFrame();
        //DrawZombieShooterHUD();
        //RenderImGui();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ////  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
 

        glfwSwapBuffers(window);
        glfwPollEvents();


        //std::cout << "Camera: "
        ssTitle << "Camera: "
            << ::g_pFlyCamera->getEyeLocation().x << ", "
            << ::g_pFlyCamera->getEyeLocation().y << ", "
            << ::g_pFlyCamera->getEyeLocation().z 
            << "   ";
        //ssTitle << "light[" << g_selectedLightIndex << "] "
        //    << ::g_pLightManager->theLights[g_selectedLightIndex].position.x << ", "
        //    << ::g_pLightManager->theLights[g_selectedLightIndex].position.y << ", "
        //    << ::g_pLightManager->theLights[g_selectedLightIndex].position.z
        //    << "   "
        //    << "linear: " << ::g_pLightManager->theLights[0].atten.y
        //    << "   "
        //    << "quad: " << ::g_pLightManager->theLights[0].atten.z
        //    << " particles:" << ::g_pParticles->GetNumberOfLiveParticles();

        //ssTitle << " BP tris: " << numberOfNarrowPhaseTrianglesInAABB_BroadPhaseThing;

        //// Add the viper info, too
        //cPhysics::sPhysInfo* pViperPhys = ::g_pPhysicEngine->pFindAssociateMeshByFriendlyName("New_Viper_Player");
        //if (pViperPhys)
        //{
        //    ssTitle
        //        << " Viper XYZ:" << ::g_getStringVec3(pViperPhys->position)
        //        << " vel:" << ::g_getStringVec3(pViperPhys->velocity)
        //        << " acc:" << ::g_getStringVec3(pViperPhys->acceleration);

        //}//if (pViperPhys)

        //// Show frame time


        ssTitle << " deltaTime = " << deltaTime
            << " FPS: " << 1.0 / deltaTime;
        /*if (pPlayerReference)
        {
            ssTitle << " Current Animation = " << pPlayerReference->pPlayerAnimator->GetCurrentAnimation();
            ssTitle << " LineTrace Distance = " << LineTrace(glm::vec3(0.0f,1.0f,0.0f), pPlayerReference->pPlayerSkeletalMesh->positionXYZ, 500.0f, program);
        }*/
        

 //       std::cout << " deltaTime = " << deltaTime << " FPS: " << 1.0 / deltaTime << std::endl;


//        glfwSetWindowTitle(window, "Hey!");
        glfwSetWindowTitle(window, ssTitle.str().c_str());


    }// End of the draw loop

    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
    

    // Delete everything
    delete ::g_pFlyCamera;
    delete ::g_pPhysicEngine;
    ::g_pFBO_G_Buffer->shutdown();
    delete ::g_pFBO_G_Buffer;

    //pPlayerReference->pControllerManager->release();
    ::g_pPhysX->cleanupPhysics(true);
    delete ::g_pPhysX;

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}










// Add object to scene through Lua
// AddMeshToScene('plyname.ply', 'friendlyName', x, y, z);
int g_Lua_AddMeshToScene(lua_State* L)
{
//    std::cout << "g_Lua_AddMeshToScene" << std::endl;

    //{
    //    sModelDrawInfo galacticaModel;
    //    ::g_pMeshManager->LoadModelIntoVAO("assets/models/Battlestar_Galactica_Res_0_(444,087 faces)_xyz_n_uv (facing +z, up +y).ply",
    //        galacticaModel, program);
    //    std::cout << galacticaModel.meshName << ": " << galacticaModel.numberOfVertices << " vertices loaded" << std::endl;
    //}

    // AddMeshToScene('plyname.ply', 'friendlyName', x, y, z);

    sMesh* pNewMesh = new sMesh();
    pNewMesh->modelFileName = lua_tostring(L, 1);       // 'plyname.ply'
    pNewMesh->uniqueFriendlyName = lua_tostring(L, 2);  // Friendly name
    pNewMesh->positionXYZ.x = (float)lua_tonumber(L, 3);
    pNewMesh->positionXYZ.y = (float)lua_tonumber(L, 4);
    pNewMesh->positionXYZ.z = (float)lua_tonumber(L, 5);
    pNewMesh->textures[0] = lua_tostring(L, 6);
    pNewMesh->blendRatio[0] = (float)lua_tonumber(L, 7);
    //
    pNewMesh->bIsVisible = true;
    ::g_vecMeshesToDraw.push_back(pNewMesh);

    return 0;
}



























//using namespace std;

void ConsoleStuff(void)
{
    // "o" for output
//    std::ofstream myFile("someData.txt");
    // Write something
    //myFile << "Hello" << std::endl;
    //myFile << "there";
    //myFile.close();

    // Now read this file
//    std::ifstream myFile2("someData.txt");
//    std::string someString;
//    myFile2 >> someString;
//    std::cout << someString << std::endl;
//
    //std::string aword;
    //while (aword != "END_OF_FILE")
    //{
    //    myFile2 >> aword;
    //    std::cout << aword << std::endl;
    //};

    //std::string aword;
    //while (myFile2 >> aword)
    //{
    //    std::cout << aword << std::endl;
    //};

    std::ifstream myFile2("assets/models/bun_zipper_res3.ply");
    if (myFile2.is_open())
    {

        std::string aword;
        while (myFile2 >> aword)
        {
            std::cout << aword << std::endl;
        };
    }
    else
    {
        std::cout << "Can't find file" << std::endl;
    }


    // iostream
    std::cout << "Type a number:" << std::endl;

    int x = 0;
    std::cin >> x;

    std::cout << "You typed: " << x << std::endl;

    std::cout << "Type your name:" << std::endl;
    std::string name;
    std::cin >> name;

    std::cout << "Hello " << name << std::endl;
    return;
}


//int& getNumber(void)
//{
//    int p = 0;
//    return p;
//}

//cTankFactory* pTankFactory = NULL;
cTankBuilder* pTheTankBuilder = NULL;

// This is here for speed 
void SetUpTankGame(void)
{
 
    ::g_pTankArena = new cArena();

    if (!pTheTankBuilder)
    {
        pTheTankBuilder = new cTankBuilder();
    }



    

    std::vector<std::string> vecTankTpyes;
//    pTankFactory->GetTankTypes(vecTankTpyes);
//    cTankFactory::get_pTankFactory()->GetTankTypes(vecTankTpyes);
    pTheTankBuilder->GetTankTypes(vecTankTpyes);
    std::cout << "The tank factory can create "
        << vecTankTpyes.size() << " types of tanks:" << std::endl;
    for (std::string tankTypeString : vecTankTpyes)
    {
        std::cout << tankTypeString << std::endl;
    }
    std::cout << std::endl;

    // Create 1 super tank
//    iTank* pTheTank = cTankFactory::get_pTankFactory()->CreateATank("Super Tank");
    iTank* pTheTank = pTheTankBuilder->CreateATank("Super Tank!");
    if (pTheTank)
    {
        ::g_vecTheTanks.push_back(pTheTank);
    }

    // Create 10 tanks
    for (unsigned int count = 0; count != 50; count++)
    {
//        iTank* pTheTank = cTankFactory::get_pTankFactory()->CreateATank("Regular Tank");
        iTank* pTheTank = pTheTankBuilder->CreateATank("Regular Tank with Shield");
        if (pTheTank)
        {
            ::g_vecTheTanks.push_back(pTheTank);
        }
    }
    
    // Also a hover tank
//    iTank* pHoverTank = cTankFactory::get_pTankFactory()->CreateATank("Hover Tank");
    iTank* pHoverTank = pTheTankBuilder->CreateATank("Hover Tank");
    if (pHoverTank)
    {
        ::g_vecTheTanks.push_back(pHoverTank);
    }



    const float WORLD_SIZE(100.0f);

    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        glm::vec3 tankLocXYZ;
        tankLocXYZ.x = ::g_getRandomFloat(-WORLD_SIZE, WORLD_SIZE);
        tankLocXYZ.y = -5.0f;
        tankLocXYZ.z = ::g_getRandomFloat(-WORLD_SIZE, WORLD_SIZE);

        pCurrentTank->setLocation(tankLocXYZ);
    }

    // Tell the tanks about the mediator
    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        pCurrentTank->setMediator(::g_pTankArena);
    }


    for (iTank* pCurrentTank : ::g_vecTheTanks)
    {
        ::g_pTankArena->AddTank(pCurrentTank);
    }

    return;
}


void TankStepFrame(double timeStep)
{



    return;
}

// x = 5, y = 15    --> 0, 1
// x = 40.0, y = 80.0   --> [4][8]
// (40.0, 80.0) --> box size = 100
//   [0][0]   
void calcBoxXYFromCoord(float x, float y, int &xIndex, int &yIndex, float boxSize)
{
    xIndex = (int)(x / boxSize);
    yIndex = (int)(y / boxSize);
    return;
}


void AABBOctTree(void)
{
    struct sSquare
    {
        //       vector< cTriangles* > vecTriangleInThisSquare
        glm::vec2 minXY;
        glm::vec2 maxXY;
        float width;
        unsigned int indexColRow;
    };

    sSquare grid[10][10];
    float sqaureWidth = 10;

    for (unsigned int x = 0; x < 10; x++)
    {
        for (unsigned int y = 0; y < 10; y++)
        {
            grid[x][y].width = sqaureWidth;
            grid[x][y].minXY.x = sqaureWidth * x;
            grid[x][y].minXY.y = sqaureWidth * y;

            grid[x][y].maxXY.x = sqaureWidth * x + sqaureWidth;
            grid[x][y].maxXY.y = sqaureWidth * y + sqaureWidth;
        }
    }

    int xIndex, yIndex;
    calcBoxXYFromCoord(5.0f, 15.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;


    calcBoxXYFromCoord(40.0f, 80.0f, xIndex, yIndex, sqaureWidth);
    std::cout << xIndex << ", " << yIndex << std::endl;




    return;
}