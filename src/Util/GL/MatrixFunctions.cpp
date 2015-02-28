// MatrixFunctions.cpp

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <OVR.h>

glm::mat4 makeChassisMatrix_glm(
    float chassisYaw,
    float chassisPitch,
    float chassisRoll,
    glm::vec3 chassisPos)
{
    return
        glm::translate(glm::mat4(1.f), chassisPos)
        * glm::rotate(glm::mat4(1.f), -chassisYaw, glm::vec3(0,1,0))
        * glm::rotate(glm::mat4(1.f), -chassisRoll, glm::vec3(0,0,1))
        * glm::rotate(glm::mat4(1.f), -chassisPitch, glm::vec3(1,0,0))
        ;
}

/// Turn ovrPosef of the HMD into a glm matrix. Pose is delivered by libOVR per-frame
/// as close to render time as possible to reduce latency.
glm::mat4 makeMatrixFromPose(const ovrPosef& eyePose, float headSize)
{
    const OVR::Vector3f& p = eyePose.Position;
    const OVR::Quatf& q = eyePose.Orientation;
    return glm::translate(glm::mat4(1.f), headSize*glm::vec3(p.x, p.y, p.z))
        * glm::mat4_cast(glm::quat(q.w, q.x, q.y, q.z));
}

///@return an equivalent OVR matrix to the given glm one.
/// OVR uses DX's left-handed convention, so transpose is necessary.
OVR::Matrix4f makeOVRMatrixFromGlmMatrix(const glm::mat4& glm_m)
{
    OVR::Matrix4f ovr_m;
    memcpy(
        reinterpret_cast<float*>(&ovr_m.M[0][0]),
        glm::value_ptr(glm::transpose(glm_m)),
        16*sizeof(float));
    return ovr_m; // copied on return
}
