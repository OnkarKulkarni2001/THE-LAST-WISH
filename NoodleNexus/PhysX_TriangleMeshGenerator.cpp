#include "PhysXShared.h"

PxTriangleMesh* CreateMyTriangleMesh(physx::PxPhysics* physics, const std::vector<physx::PxVec3>& vertices, const std::vector<uint32_t>& indices) {
    // Prepare the triangle mesh description
    physx::PxTriangleMeshDesc meshDesc;
    meshDesc.points.count = static_cast<uint32_t>(vertices.size());
    meshDesc.points.stride = sizeof(physx::PxVec3);
    meshDesc.points.data = vertices.data();

    meshDesc.triangles.count = static_cast<uint32_t>(indices.size()) / 3;
    meshDesc.triangles.stride = 3 * sizeof(uint32_t);
    meshDesc.triangles.data = indices.data();

    // Prepare cooking parameters
    PxCookingParams params = PxTolerancesScale();
    //params.smallMeshThreshold = 100.0f; // Example threshold, adjust as needed

    // Prepare an output stream to hold the cooked data
    PxDefaultMemoryOutputStream writeBuffer;

    // Optional: Set up a result variable to check the cooking result
    PxTriangleMeshCookingResult::Enum result;

    // Cook the triangle mesh
    bool success = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);

    // Check if cooking was successful
    if (!success) {
        // Handle the error if cooking fails
        return nullptr;
    }

    // Read the cooked data into a buffer and create the triangle mesh
    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    return physics->createTriangleMesh(readBuffer);
}

PxConvexMesh* CreateMyConvexMesh(PxPhysics* physics, const std::vector<PxVec3>& vertices) {
    // Convert your vertices into a PhysX-compatible format and create the convex mesh
    PxConvexMeshDesc convexDesc;
    convexDesc.points.count = vertices.size();
    convexDesc.points.data = vertices.data();
    convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxCookingParams params = PxTolerancesScale();
    PxDefaultMemoryOutputStream buf;
    PxConvexMeshCookingResult::Enum result;
    bool success = PxCookConvexMesh(params, convexDesc, buf);
    if (!success) {
        return nullptr; // Handle failure
    }

    PxDefaultMemoryInputData inputData(buf.getData(), buf.getSize());
    return physics->createConvexMesh(inputData);
}

void createStaticCube(const PxVec3& position, const PxVec3& halfExtents, const glm::vec3& rotation)
{
    // Create a material (friction, restitution)
    PxMaterial* material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

    // Define the shape of the wall (box)
    PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtents), *material);

    // Define transform with rotation
    PxTransform transform(position, PxQuat(glm::radians(rotation.y), PxVec3(0, 1, 0)));

    // Create static rigid body (it won’t move)
    PxRigidStatic* cubeActor = gPhysics->createRigidStatic(transform);
    cubeActor->attachShape(*shape);

    // Add actor to the scene
    gScene->addActor(*cubeActor);

    // Release shape (safe to release after attaching)
    shape->release();
}