// MatrixFunctions.cpp

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef USE_OCULUSSDK
#include <OVR_CAPI.h>
#endif

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

#ifdef USE_OCULUSSDK
/// Turn ovrPosef of the HMD into a glm matrix. Pose is delivered by libOVR per-frame
/// as close to render time as possible to reduce latency.
glm::mat4 makeMatrixFromPose(const ovrPosef& eyePose, float headSize)
{
    const ovrVector3f& p = eyePose.Position;
    const ovrQuatf& q = eyePose.Orientation;
    return glm::translate(glm::mat4(1.f), glm::vec3(p.x, p.y, p.z))
        * glm::mat4_cast(glm::quat(q.w, q.x, q.y, q.z));
}

glm::mat4 makeGlmMatrixFromOvrMatrix(const ovrMatrix4f& om)
{
    glm::mat4 gm;
    memcpy(
        glm::value_ptr(gm),
        reinterpret_cast<const float*>(&om.M[0][0]),
        16 * sizeof(float));
    return glm::transpose(gm);
}

///@return an equivalent OVR matrix to the given glm one.
/// OVR uses DX's left-handed convention, so transpose is necessary.
ovrMatrix4f makeOVRMatrixFromGlmMatrix(const glm::mat4& glm_m)
{
    ovrMatrix4f ovr_m;
    memcpy(
        reinterpret_cast<float*>(&ovr_m.M[0][0]),
        glm::value_ptr(glm::transpose(glm_m)),
        16 * sizeof(float));
    return ovr_m; // copied on return
}

///@return the unit vector along negative z transformed by the given pose
void GetHMDEyeRayPosAndDir(const ovrPosef& pose, glm::vec3& ro, glm::vec3& rd)
{
    const glm::mat4 poseMtx = makeMatrixFromPose(pose, 1.f);
    const glm::vec4 origin(0.f, 0.f, 0.f, 1.f);
    const glm::vec4 lookFwd(0.f, 0.f, -1.f, 0.f);
    const glm::vec4 o4 = poseMtx * origin;
    const glm::vec4 d4 = poseMtx * lookFwd;
    ro = glm::vec3(o4.x, o4.y, o4.z);
    rd = glm::vec3(d4.x, d4.y, d4.z);
}
#endif
