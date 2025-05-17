#include "cFBO/cFBO_RGB_depth.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> 
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr
#include "cBasicFlyCamera/cBasicFlyCamera.h"
#include "sMesh.h"
#include "cBasicTextureManager/cBasicTextureManager.h"
#include "cFBO/cFBO_deferred.h"
#include "cPlayer.h"

extern cPlayer* pPlayerReference;
extern std::vector<sMesh*> vecCeilings;
//float g_orthoSize = 10.0f;
extern cVAOManager* g_pMeshManager;

extern double deltaTime;
void RenderAllAnimated(GLuint& program, glm::mat4& matProjection, glm::mat4& matView, float deltaTime);
void RenderScene(
    GLuint program,
    glm::mat4 matProjection,
    glm::mat4 matView,
    float ratio,
    glm::vec3 eyeLocation);

void DrawMesh(
    sMesh* pCurMesh,
    glm::mat4 matModel,         // "parent" or initial model matrix
    GLuint program,
    bool SetTexturesFromMeshInfo = true);

sMesh* g_pFindMeshByFriendlyName(std::string theNameToFind);

extern cBasicFlyCamera* g_pFlyCamera;
extern cBasicTextureManager* g_pTextures;
extern cFBO_deferred* g_pFBO_G_Buffer;

void FBOInit(cFBO_RGB_depth& FBO_View) {
    std::string FBOError;
    if (!FBO_View.init(1920, 1080, FBOError))
        //    if (!FBO_WarehouseView.init(128, 64, FBOError))
        //    if (!FBO_WarehouseView.init(3840 * 2, 2180 * 2, FBOError))
    {
        std::cout << "Error: FBO.init(): " << FBOError << std::endl;
    }
    else
    {
        std::cout << "FBO created OK" << std::endl;
    }
}

void FBOBind(cFBO_RGB_depth& FBO_View, float ratio, GLuint program, glm::vec3 eyeLocation, glm::vec3 targetLocation, float orthoSize) {

    //sMesh* skySphere = g_pFindMeshByFriendlyName("SkySphere");
    //skySphere->bIsVisible = false;

    glm::mat4 matProjection = glm::mat4(1.0f);
    glm::mat4 matView = glm::mat4(1.0f);
    glm::vec3 upVector = glm::vec3(0.0f, 0.0f, -1.0f);

    //**********************************************************topdown*********************************
    //orthoSize = 5.0f;  // Adjust based on how zoomed out you want it
    matProjection = glm::ortho(
        -orthoSize * ratio, orthoSize * ratio,
        -orthoSize, orthoSize,
        0.1f, 1000.0f
    );

    // Assume these come from your player object
    glm::vec3 playerPosition = pPlayerReference->pPlayerSkeletalMesh->positionXYZ;
    glm::vec3 forward = pPlayerReference->getPlayerForward(); // or: playerRotation * glm::vec3(0, 0, 1);

    //eyeLocation.y = orthoSize;
    // Set camera position above the player
    glm::vec3 newEyeLocation = playerPosition + eyeLocation;  // Y-up world, 200 units above
    glm::vec3 newTargetLocation = playerPosition;

    // Compute "right" and use it as 'up' for the view
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 cameraUp = right;

    // Set the view matrix
    matView = glm::lookAt(newEyeLocation, newTargetLocation, cameraUp);
    //**********************************************************topdown*********************************

    // **************************************************
    // 
    // RENDER from the inside of the warehouse
    GLint renderPassNumber_UL = glGetUniformLocation(program, "renderPassNumber");
    glUniform1i(renderPassNumber_UL, 0);

    // Point output to the off-screen FBO
    glBindFramebuffer(GL_FRAMEBUFFER, FBO_View.ID);

    // These will ONLY work on the default framebuffer
       // glViewport(0, 0, width, height);
       //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

       // FBO_WarehouseView.clearColourBuffer(0);
    FBO_View.clearBuffers(true, true);


    //matProjection = glm::perspective(0.6f,
    //    ratio,
    //    0.1f,
    //    1000.0f);

    //glm::vec3 eyeInsideWarehouse = glm::vec3(-197.0f, 14.0f, -72.0f);
    //float xOffset = 10.0f * glm::sin((float)glfwGetTime() / 2.0f);
    //glm::vec3 atInsideWareHouse =
    //    eyeInsideWarehouse + glm::vec3(xOffset, 0.0f, 10.0f);

    //matView = glm::lookAt(eyeLocation, targetLocation, glm::vec3(0.0f, 0.0f, -1.0f));
    glUseProgram(program);
    //std::cout << "Eye: " << eyeLocation.x << ", " << eyeLocation.y << ", " << eyeLocation.z << std::endl;
    //std::cout << "Target: " << targetLocation.x << ", " << targetLocation.y << ", " << targetLocation.z << std::endl;


    GLint texture01_UL = glGetUniformLocation(program, "texture00");
    glUniform1i(texture01_UL, 0);
    
    for (sMesh* pCurCeiling : vecCeilings) {
        pCurCeiling->bIsVisible = false;
    }

    RenderScene(program, matProjection, matView, ratio, eyeLocation);

    for (sMesh* pCurCeiling : vecCeilings) {
        pCurCeiling->bIsVisible = true;
    }
    //skySphere->bIsVisible = true;1
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glBindFramebuffer(GL_FRAMEBUFFER, g_pFBO_G_Buffer->ID);
}

void RenderFBOTextureSingleMesh(sMesh* pFBOTextureMesh, cFBO_RGB_depth& FBO_View, GLuint program, glm::mat4& matProjection, glm::mat4& matView, float ratio)
{
    //glBindFramebuffer(GL_FRAMEBUFFER, g_pFBO_G_Buffer->ID); // Render to G-buffer
    //glBindFramebuffer(GL_FRAMEBUFFER, 0); // Render to G-buffer

    if (pFBOTextureMesh)
    {
        pFBOTextureMesh->bIsVisible = true;

        glUseProgram(program);

        GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        GLint matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);



        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, FBO_View.colourTexture_0_ID);

        GLint texture01_UL = glGetUniformLocation(program, "texture00");
        //GLint texture01_UL = glGetUniformLocation(program, "FBOTexture");
        glUniform1i(texture01_UL, 0);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here

        GLint texRatio_0_to_3_UL = glGetUniformLocation(program, "texRatio_0_to_3");
        glUniform4f(texRatio_0_to_3_UL, 1.0f, 0.0f, 0.0f, 0.0f);

        GLint b_Is_FBO_Texture_UL = glGetUniformLocation(program, "b_Is_FBO_Texture");
        //GLint bUseTextureAsColour_UL = glGetUniformLocation(program, "bUseTextureAsColour");

        glUniform1f(b_Is_FBO_Texture_UL, (GLfloat)GL_TRUE);
        //glUniform1f(bUseTextureAsColour_UL, (GLfloat)GL_TRUE); // Use FBO texture
        
        DrawMesh(pFBOTextureMesh, program, false);

        glActiveTexture(GL_TEXTURE0 + 18);
        glBindTexture(GL_TEXTURE_2D, FBO_View.colourTexture_0_ID);
        GLint FBO_UL
            = glGetUniformLocation(program, "FBOTexture");
        glUniform1i(FBO_UL, 18);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here

        //glUniform1f(bUseTextureAsColour_UL, (GLfloat)GL_FALSE);
        //glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
        //pFBOTextureMesh->bIsVisible = false;
    }
}

void RenderMinimapWithScreenspaceShader(sMesh* pQuadMesh, GLuint minimapTextureID, GLuint minimapShaderProgram, glm::mat4& matProjection, glm::mat4& matView, float ratio)
{
    if (pQuadMesh)
    {
        pQuadMesh->bIsVisible = true;

        // Disable depth testing for the minimap (optional, depending on your needs)
        glDisable(GL_DEPTH_TEST);

        // Save current viewport
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        // Set viewport to top-right corner (20% of screen size)
        glViewport(viewport[2] * 0.75f, viewport[3] * 0.75f, viewport[2] * 0.2f, viewport[3] * 0.2f);

        // Use the minimap shader program (3D shader)
        glUseProgram(minimapShaderProgram);

        // Set up orthographic projection for the minimap
        glm::mat4 orthoProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
        // Adjust ortho parameters based on your quad mesh size
        // Example: If quad is 2x2 units, use glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, ...)

        // Set up view matrix (camera looking down at the quad)
        glm::mat4 minimapView = glm::lookAt(
            glm::vec3(0.0f, 10.0f, 0.0f), // Camera above the quad
            glm::vec3(0.0f, 0.0f, 0.0f),  // Look at quad’s center
            glm::vec3(0.0f, 0.0f, 1.0f)   // Up vector
        );

        // Model matrix (identity or adjust for quad positioning/scaling)
        glm::mat4 model = glm::mat4(1.0f);
        // Example: Scale down if quad is too large
        // model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

        // Pass matrices to shader
        glUniformMatrix4fv(glGetUniformLocation(minimapShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProjection));
        glUniformMatrix4fv(glGetUniformLocation(minimapShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(minimapView));
        glUniformMatrix4fv(glGetUniformLocation(minimapShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, minimapTextureID);
        glUniform1i(glGetUniformLocation(minimapShaderProgram, "textureMap"), 0);
        glUniform1i(glGetUniformLocation(minimapShaderProgram, "useTexture"), 1);


        // Draw the quad mesh
        sModelDrawInfo meshToDrawInfo;
        if (::g_pMeshManager->FindDrawInfoByModelName("assets/models/2x2_Quad_for_FSQuad_xyz_n_uv.ply", meshToDrawInfo))
        {
            glBindVertexArray(meshToDrawInfo.VAO_ID);
            glDrawElements(GL_TRIANGLES, meshToDrawInfo.numberOfIndices, GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
        }

        // Unbind shader
        glUseProgram(0);

        // Restore original viewport
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        // Re-enable depth testing
        glEnable(GL_DEPTH_TEST);
    }
}



void RenderFBOTexture(sMesh* pFBOTextureMesh, sMesh* pFBOTextureMesh2, cFBO_RGB_depth& FBO_View, cFBO_RGB_depth& FBO_View2, GLuint program, glm::mat4& matProjection, glm::mat4& matView, float ratio) {

    if (pFBOTextureMesh && pFBOTextureMesh2)
    {
        pFBOTextureMesh->bIsVisible = true;
        pFBOTextureMesh2->bIsVisible = true;

        GLint matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        GLint matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);



        // Connect texture unit #0 to the offscreen FBO
        glActiveTexture(GL_TEXTURE0);

        // The colour texture inside the FBO is just a regular colour texture.
        // There's nothing special about it.
        glBindTexture(GL_TEXTURE_2D, FBO_View.colourTexture_0_ID);
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
        //GLint bUseTextureAsColour_UL = glGetUniformLocation(program, "bUseTextureAsColour");

        DrawMesh(pFBOTextureMesh, program, false);
        pFBOTextureMesh->bIsVisible = false;

        matProjection_UL = glGetUniformLocation(program, "matProjection");
        glUniformMatrix4fv(matProjection_UL, 1, GL_FALSE, (const GLfloat*)&matProjection);

        matView_UL = glGetUniformLocation(program, "matView");
        glUniformMatrix4fv(matView_UL, 1, GL_FALSE, (const GLfloat*)&matView);



        // Connect texture unit #0 to the offscreen FBO
        glActiveTexture(GL_TEXTURE0);

        // The colour texture inside the FBO is just a regular colour texture.
        // There's nothing special about it.
        glBindTexture(GL_TEXTURE_2D, FBO_View2.colourTexture_0_ID);
        //glBindTexture(GL_TEXTURE_2D, ::g_pTextures->getTextureIDFromName("dua-lipa-promo.bmp"));

        texture01_UL = glGetUniformLocation(program, "FBOTexture");
        glUniform1i(texture01_UL, 0);       // <-- Note we use the NUMBER, not the GL_TEXTURE3 here

        texRatio_0_to_3_UL = glGetUniformLocation(program, "texRatio_0_to_3");
        glUniform4f(texRatio_0_to_3_UL,
            1.0f,
            0.0f,
            0.0f,
            0.0f);

        GLint b_Is_FBO_Texture_UL = glGetUniformLocation(program, "b_Is_FBO_Texture");
        //GLint bUseTextureAsColour_UL = glGetUniformLocation(program, "bUseTextureAsColour");
        DrawMesh(pFBOTextureMesh2, program, false);

        glUniform1f(b_Is_FBO_Texture_UL, (float)GL_FALSE);
        //glUniform1f(bUseTextureAsColour_UL, (float)GL_TRUE);

        pFBOTextureMesh2->bIsVisible = false;
    }


    //RenderScene(program, matProjection, matView, ratio, ::g_pFlyCamera->getEyeLocation());
}