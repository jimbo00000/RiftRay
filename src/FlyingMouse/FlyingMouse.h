// FlyingMouse.h

#pragma once

#include <glm/glm.hpp>

#ifdef USE_SIXENSE
  #include <sixense.h>
  #include <sixense_utils/controller_manager/controller_manager.hpp>
#else

typedef struct _sixenseControllerData {
  float pos[3];
  float rot_mat[3][3];
  float joystick_x;
  float joystick_y;
  float trigger;
  unsigned int buttons;
  unsigned char sequence_number;
  float rot_quat[4];
  unsigned short firmware_revision;
  unsigned short hardware_revision;
  unsigned short packet_type;
  unsigned short magnetic_frequency;
  int enabled;
  int controller_index;
  unsigned char is_docked;
  unsigned char which_hand;
  unsigned char hemi_tracking_enabled;
} sixenseControllerData;

typedef struct _sixenseAllControllerData {
  sixenseControllerData controllers[4];
} sixenseAllControllerData;

#endif // USE_SIXENSE


class FlyingMouse
{
public:
    enum Hand
    {
        Right=0, // Match Sixense SDK constants
        Left=1,
    };

    FlyingMouse();
    virtual ~FlyingMouse();

    void Init();
    void Destroy();
    void updateHydraData();

    bool IsActive() const { return m_active; }
    sixenseAllControllerData GetCurrentState() const { return g_curAcd; }

    bool IsPressed(Hand h, int buttonID) const;
    bool WasJustPressed(int buttonID) const;
    bool WasJustPressed(Hand h, int buttonID) const;
    bool WasJustReleased(Hand h, int buttonID) const;

    bool TriggerCrossedThreshold(Hand h, float thresh) const;
    bool TriggerIsOverThreshold(Hand h, float thresh) const;
    float GetTriggerValue(Hand h) const;

    void GetControllerOriginAndDirection(Hand h, glm::vec3& origin, glm::vec3& direction) const;
    bool ControllerIsOnBase(Hand h) const;
    void SetChassisPosPointer(glm::vec3* pPos) { m_pChassisPos = pPos; }
    void SetChassisYawPointer(float* pYaw) { m_pChassisYaw = pYaw; }
    glm::vec3 GetChassisPos() const { return *m_pChassisPos; }

    bool m_active;
    float mtxL[16];
    float mtxR[16];

    glm::vec3 m_baseOffset;

protected:
    sixenseAllControllerData g_curAcd;
    sixenseAllControllerData g_lastAcd;
    glm::vec3* m_pChassisPos;
    float* m_pChassisYaw;

private: // Disallow copy ctor and assignment operator
    FlyingMouse(const FlyingMouse&);
    FlyingMouse& operator=(const FlyingMouse&);
};
