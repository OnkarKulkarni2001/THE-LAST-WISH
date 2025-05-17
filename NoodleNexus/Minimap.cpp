#include "glm/glm.hpp"
#include "GLCommon.h"
#include "cFBO/cFBO_deferred.h"
#include <iostream>
#include "sMesh.h"
#include "cVAOManager/cVAOManager.h"
#include "cMasterZombie.h"

extern GLuint program;
extern cFBO_deferred* g_pFBO_G_Buffer;
extern GLFWwindow* window;
extern cVAOManager* g_pMeshManager;

void RenderMinimap(GLuint minimapShaderProgram, GLuint minimapTextureID) {
    // Use the minimap shader program
    glUseProgram(minimapShaderProgram);

    // Pass the offset (top-right corner of the screen)
    glUniform2f(glGetUniformLocation(minimapShaderProgram, "uOffset"), 0.75f, 0.75f);

    // Pass the scale (size of the minimap)
    glUniform2f(glGetUniformLocation(minimapShaderProgram, "uScale"), 0.2f, 0.2f);

    // Bind the minimap texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, minimapTextureID);
    glUniform1i(glGetUniformLocation(minimapShaderProgram, "minimapTexture"), 0);

    // Draw the quad (assuming the VAO for the minimap is already set up)
    sModelDrawInfo meshToDrawInfo;
    if (::g_pMeshManager->FindDrawInfoByModelName("assets/models/2x2_Quad_for_FSQuad_xyz_n_uv.ply", meshToDrawInfo))
    {
        // Found the model
        glBindVertexArray(meshToDrawInfo.VAO_ID); 		// enable VAO(and everything else)
        //https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
        glDrawElements(GL_TRIANGLES,
            meshToDrawInfo.numberOfIndices,
            GL_UNSIGNED_INT,
            (void*)0);

        glBindVertexArray(0); 			//disable VAO(and everything else)
    }
    glUseProgram(0);     // Unbind shader

}

struct Ray
{
    glm::vec3 origin;
    glm::vec3 direction;
};

// Get ray from camera through screen point
Ray GetCameraRay(float screenX, float screenY, float screenWidth, float screenHeight,
    const glm::mat4& viewMatrix, const glm::mat4& projMatrix)
{
    // Screen to NDC
    float x = (2.0f * screenX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * screenY) / screenHeight;

    // Inverse view-projection
    glm::mat4 invViewProj = glm::inverse(projMatrix * viewMatrix);

    // Near and far points in world space
    glm::vec4 nearPoint(x, y, 0.0f, 1.0f);
    glm::vec4 farPoint(x, y, 1.0f, 1.0f);

    nearPoint = invViewProj * nearPoint;
    farPoint = invViewProj * farPoint;

    if (nearPoint.w != 0.0f) nearPoint /= nearPoint.w;
    if (farPoint.w != 0.0f) farPoint /= farPoint.w;

    Ray ray;
    ray.origin = glm::vec3(nearPoint);
    ray.direction = glm::normalize(glm::vec3(farPoint - nearPoint));
    return ray;
}

// Example raycast to find hit point
glm::vec3 GetCrosshairWorldPos(float screenWidth, float screenHeight, const glm::mat4& viewMatrix,
    const glm::mat4& projMatrix, std::vector<cMasterZombie*> AllZombies, int numZombies)
{
    float crosshairX = screenWidth / 2.0f;
    float crosshairY = screenHeight / 2.0f;

    Ray ray = GetCameraRay(crosshairX, crosshairY, screenWidth, screenHeight, viewMatrix, projMatrix);

    // Simple raycast: Check intersection with zombie heads (e.g., spheres)
    glm::vec3 hitPos = ray.origin + ray.direction * 10.0f; // Default: 10 units
    float minDist = std::numeric_limits<float>::max();

    for (int i = 0; i < numZombies; ++i)
    {
        glm::vec3 headPos = AllZombies[i]->pZombieSkeletalMesh->positionXYZ + glm::vec3(0.0f, 1.7f, 0.0f);
        float radius = 0.5f; // Head radius

        // Ray-sphere intersection
        glm::vec3 oc = ray.origin - headPos;
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.0f * glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - 4 * a * c;

        if (discriminant >= 0)
        {
            float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
            if (t > 0 && t < minDist)
            {
                minDist = t;
                hitPos = ray.origin + ray.direction * t;
            }
        }
    }

    return hitPos;
}

glm::vec3 ScreenToWorld(const glm::vec3& screenPos, const glm::mat4& view, const glm::mat4& proj, int screenWidth, int screenHeight)
{
    glm::mat4 invVP = glm::inverse(proj * view);

    // Convert screen coordinates back to NDC
    glm::vec4 ndc;
    ndc.x = (screenPos.x / screenWidth) * 2.0f - 1.0f;
    ndc.y = 1.0f - (screenPos.y / screenHeight) * 2.0f;
    ndc.z = screenPos.z;  // z should be between 0.0 (near) and 1.0 (far)
    ndc.w = 1.0f;

    glm::vec4 world = invVP * ndc;
    world /= world.w;

    return glm::vec3(world);
}

glm::vec3 WorldToScreen(glm::vec3& worldPos, glm::mat4& view, glm::mat4& proj, int screenWidth, int screenHeight)
{
    glm::vec4 clipSpacePos = proj * view * glm::vec4(worldPos, 1.0f);

    if (clipSpacePos.w == 0.0f)
        return glm::vec3(0.0f); // Avoid division by zero

    glm::vec3 ndc = glm::vec3(clipSpacePos) / clipSpacePos.w; // Perspective divide

    // Convert NDC [-1, 1] to screen coordinates
    glm::vec3 screen;
    screen.x = (ndc.x + 1.0f) * 0.5f * screenWidth;
    screen.y = (1.0f - ndc.y) * 0.5f * screenHeight; // Flip Y for screen space
    screen.z = ndc.z;

    return screen;
}


glm::vec3 screenToWorldBottomRight(glm::mat4 proj, glm::mat4 view, float screenWidth, float screenHeight)
{
    // Bottom right in NDC space: (1, -1)
    //glm::vec4 ndc = glm::vec4(1.0f, -1.0f, 1.0f, 1.0f); // z = 1 for far plane, or 0.1 for near-ish

    //glm::mat4 invVP = glm::inverse(proj * view);
    //glm::vec4 world = invVP * ndc;
    //world /= world.w;
    glm::vec3 screenPos(screenWidth - 50.0f, screenHeight - 50.0f, 0.1f); // z = 0.1 near front
    glm::vec3 worldPos = ScreenToWorld(screenPos, view, proj, screenWidth, screenHeight);

    return glm::vec3(worldPos);
}

GLuint debugLineVAO = 0, debugLineVBO = 0;

struct sDebugLineDrawInfo {
    GLuint VAO_ID = 0;
    GLuint VertexBufferID = 0;
    bool bIsCopiedToVAO = false;
};

sDebugLineDrawInfo g_DebugLineInfo;

void InitDebugLineVAO(GLuint shaderProgramID)
{
    glUseProgram(shaderProgramID);
    glGenVertexArrays(1, &g_DebugLineInfo.VAO_ID);
    glBindVertexArray(g_DebugLineInfo.VAO_ID);

    glGenBuffers(1, &g_DebugLineInfo.VertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, g_DebugLineInfo.VertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, nullptr, GL_DYNAMIC_DRAW); // 2 vertices * (3 pos + 3 color)

    // Get attribute locations
    GLint aPos_location = glGetAttribLocation(shaderProgramID, "aPos");
    GLint aColor_location = glGetAttribLocation(shaderProgramID, "aColor");

    if (aPos_location == -1 || aColor_location == -1) {
        std::cout << "Error: Attribute not found: aPos=" << aPos_location
            << ", aColor=" << aColor_location << std::endl;
        return;
    }

    // Vertex position attribute
    glEnableVertexAttribArray(aPos_location);
    glVertexAttribPointer(aPos_location, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)0);

    // Vertex color attribute
    glEnableVertexAttribArray(aColor_location);
    glVertexAttribPointer(aColor_location, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)sizeof(glm::vec3));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    g_DebugLineInfo.bIsCopiedToVAO = true;
}

//void InitDebugLineVAO(GLuint shaderProgramID)
//{
//    glUseProgram(shaderProgramID);
//    glGenVertexArrays(1, &g_DebugLineInfo.VAO_ID);
//    glBindVertexArray(g_DebugLineInfo.VAO_ID);
//
//    glGenBuffers(1, &g_DebugLineInfo.VertexBufferID);
//    glBindBuffer(GL_ARRAY_BUFFER, g_DebugLineInfo.VertexBufferID);
//
//    // Define vertex data with positions and colors
//    glm::vec3 lineVertices[2] = {
//        glm::vec3(0.0f, 0.0f, 0.0f), // Start point of the line
//        glm::vec3(1.0f, 1.0f, 1.0f)  // End point of the line
//    };
//    glm::vec3 lineColors[2] = {
//        glm::vec3(1.0f, 0.0f, 0.0f), // Red color for start point
//        glm::vec3(0.0f, 1.0f, 0.0f)  // Green color for end point
//    };
//
//    // Combine position and color data into a single array for the vertex buffer
//    struct VertexData {
//        glm::vec3 position;
//        glm::vec3 color;
//    };
//
//    VertexData vertices[2] = {
//        {lineVertices[0], lineColors[0]}, // Start point with color red
//        {lineVertices[1], lineColors[1]}  // End point with color green
//    };
//
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
//
//    // Get location of attributes in the shader program
//    GLint aPos_location = glGetAttribLocation(shaderProgramID, "aPos");
//    GLint aColor_location = glGetAttribLocation(shaderProgramID, "aColor");
//
//    // Vertex position attribute
//    glEnableVertexAttribArray(aPos_location);
//    glVertexAttribPointer(aPos_location, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
//    
//    // Vertex color attribute
//    glEnableVertexAttribArray(aColor_location);
//    glVertexAttribPointer(aColor_location, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, color));
//
//    glBindVertexArray(0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//    g_DebugLineInfo.bIsCopiedToVAO = true;
//}


//void DrawDebugLineModern(const glm::vec3& start, const glm::vec3& end, glm::vec3& lineColor,
//    const glm::mat4& view, const glm::mat4& projection,
//    GLuint shaderProgram)
//{
//    if (!g_DebugLineInfo.bIsCopiedToVAO)
//    {
//        std::cout << "Debug line VAO not initialized!\n";
//        return;
//    }
//
//    glm::vec3 lineVertices[2] = { start, end };
//
//    glUseProgram(shaderProgram);
//
//    // Upload uniforms
//    GLint uViewLoc = glGetUniformLocation(shaderProgram, "uView");
//    GLint uProjLoc = glGetUniformLocation(shaderProgram, "uProj");
//    //GLint uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
//
//    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &view[0][0]);
//    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, &projection[0][0]);
//    //glUniform3fv(uColorLoc, 1, &color[0]);
//
//    // Bind VAO and update vertex data
//    glBindVertexArray(g_DebugLineInfo.VAO_ID);
//    glBindBuffer(GL_ARRAY_BUFFER, g_DebugLineInfo.VertexBufferID);
//    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(lineVertices), lineVertices);
//
//    glDrawArrays(GL_LINES, 0, 2);
//
//    glBindVertexArray(0);
//    glUseProgram(0);
//}
void DrawDebugLineModern(const glm::vec3& start, const glm::vec3& end,
    const glm::mat4& view, const glm::mat4& projection,
    GLuint shaderProgram)
{
    if (!g_DebugLineInfo.bIsCopiedToVAO)
    {
        std::cout << "Debug line VAO not initialized!\n";
        return;
    }

    // Define vertex data (positions and colors)
    struct VertexData {
        glm::vec3 position;
        glm::vec3 color;
    };

    VertexData vertices[2] = {
        {start, glm::vec3(1.0f, 0.0f, 0.0f)}, // Start: red
        {end, glm::vec3(0.0f, 1.0f, 0.0f)}    // End: green
    };

    glUseProgram(shaderProgram);

    // Get uniform locations
    GLint uViewLoc = glGetUniformLocation(shaderProgram, "uView");
    GLint uProjLoc = glGetUniformLocation(shaderProgram, "uProj");

    //std::cout << "Uniform locations: uView=" << uViewLoc
    //    << ", uProj=" << uProjLoc << std::endl;

    if (uViewLoc == -1 || uProjLoc == -1) {
        std::cout << "One or more uniforms not found!\n";
        glUseProgram(0);
        return;
    }

    // Upload uniforms
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, &projection[0][0]);

    // Bind VAO and update vertex data
    glBindVertexArray(g_DebugLineInfo.VAO_ID);
    glBindBuffer(GL_ARRAY_BUFFER, g_DebugLineInfo.VertexBufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
    glUseProgram(0);
}
