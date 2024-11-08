#define  _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory>

#include "uevr/Plugin.hpp"

#define KEYDOWN false
#define KEYUP true

typedef struct _HAPTIC_TIMER_STRUCT
{
    const UEVR_VRData* vr;
    int MillisecondsDelay;
}   HAPTIC_TIMER_STRUCT;

void DebugPrint(char* Format, ...);
using namespace uevr;

#define PLUGIN_LOG_ONCE(...) \
    static bool _logged_ = false; \
    if (!_logged_) { \
        _logged_ = true; \
        API::get()->log_info(__VA_ARGS__); \
    }
/*
OpenXR Action Paths 
    static const inline std::string s_action_pose = "/actions/default/in/Pose";
    static const inline std::string s_action_grip_pose = "/actions/default/in/GripPose";
    static const inline std::string s_action_trigger = "/actions/default/in/Trigger";
    static const inline std::string s_action_grip = "/actions/default/in/Grip";
    static const inline std::string s_action_joystick = "/actions/default/in/Joystick";
    static const inline std::string s_action_joystick_click = "/actions/default/in/JoystickClick";

    static const inline std::string s_action_a_button_left = "/actions/default/in/AButtonLeft";
    static const inline std::string s_action_b_button_left = "/actions/default/in/BButtonLeft";
    static const inline std::string s_action_a_button_touch_left = "/actions/default/in/AButtonTouchLeft";
    static const inline std::string s_action_b_button_touch_left = "/actions/default/in/BButtonTouchLeft";

    static const inline std::string s_action_a_button_right = "/actions/default/in/AButtonRight";
    static const inline std::string s_action_b_button_right = "/actions/default/in/BButtonRight";
    static const inline std::string s_action_a_button_touch_right = "/actions/default/in/AButtonTouchRight";
    static const inline std::string s_action_b_button_touch_right = "/actions/default/in/BButtonTouchRight";

    static const inline std::string s_action_dpad_up = "/actions/default/in/DPad_Up";
    static const inline std::string s_action_dpad_right = "/actions/default/in/DPad_Right";
    static const inline std::string s_action_dpad_down = "/actions/default/in/DPad_Down";
    static const inline std::string s_action_dpad_left = "/actions/default/in/DPad_Left";
    static const inline std::string s_action_system_button = "/actions/default/in/SystemButton";
    static const inline std::string s_action_thumbrest_touch_left = "/actions/default/in/ThumbrestTouchLeft";
    static const inline std::string s_action_thumbrest_touch_right = "/actions/default/in/ThumbrestTouchRight";

*/


class RemapPlugin : public uevr::Plugin {
public:
    HAPTIC_TIMER_STRUCT m_Timer;
    const UEVR_PluginInitializeParam* m_Param;
    const UEVR_VRData* m_VR;
	
    bool m_Zooming;
	bool m_OpenXr;
	bool m_OpenVr;
    bool m_IndexIndicatorActive;
    RemapPlugin() = default;
    
    void on_dllmain(HANDLE handle) override {
    }

    void on_initialize() override {
      OutputDebugString("Initializing Index-Haptic\n");
      m_IndexIndicatorActive = false;
         
      // This shows how to get to the API functions.
      m_Param = API::get()->param();
      m_VR = m_Param->vr;

      ZeroMemory(&m_Timer, sizeof(HAPTIC_TIMER_STRUCT));
      m_Timer.vr = m_VR;
      m_Timer.MillisecondsDelay = 600;
      
      m_OpenXr = m_VR->is_openxr();
      m_OpenVr = m_VR->is_openvr();
      API::get()->log_info("****************Plugin init: openxr: %d, openvr: %d\n", m_OpenXr, m_OpenVr);
    }

    void on_pre_engine_tick(UEVR_UGameEngineHandle engine, float delta) override {
        PLUGIN_LOG_ONCE("Pre Engine Tick: %f", delta);
    }

    void on_post_engine_tick(UEVR_UGameEngineHandle engine, float delta) override {
        PLUGIN_LOG_ONCE("Post Engine Tick: %f", delta);
    }

    void on_pre_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
        PLUGIN_LOG_ONCE("Pre Slate Draw Window");
    }

    void on_post_slate_draw_window(UEVR_FSlateRHIRendererHandle renderer, UEVR_FViewportInfoHandle viewport_info) override {
        PLUGIN_LOG_ONCE("Post Slate Draw Window");
    }
	
    void send_key(WORD key, bool key_up) {
        INPUT input;
        ZeroMemory(&input, sizeof(INPUT));
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        if(key_up) input.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    //*******************************************************************************************
    // This is the controller input routine. Everything happens here.
    //*******************************************************************************************
    void on_xinput_get_state(uint32_t* retval, uint32_t user_index, XINPUT_STATE* state) {
		
		if(state == NULL) return;
        if(!m_VR->is_using_controllers()) return; // If not using controllers, none of this applies.
        
        UEVR_InputSourceHandle LeftController = m_VR->get_left_joystick_source();
        UEVR_InputSourceHandle RightController = m_VR->get_right_joystick_source();
        static bool LeftGripDown = false;
        static bool RightGripDown = false;
        static bool StartDown = false;
        static bool SelectDown = false;
        static bool F1Down = false;
        static bool F2Down = false;
        static bool F3Down = false;
        static bool F4Down = false;
        static bool F5Down = false;
        static bool F6Down = false;
        static bool F7Down = false;
        static bool LTDown = false;
		static bool RBDown = false;
        static bool RBShiftDown = false;
        static bool SwapLtRb = false;
        
        bool LeftShifting = false;

		if (m_OpenXr == true)
		{
            UEVR_ActionHandle GripButton = m_VR->get_action_handle("/actions/default/in/Grip");
            UEVR_ActionHandle ATouchLeft = m_VR->get_action_handle("/actions/default/in/AButtonTouchLeft");
            UEVR_ActionHandle BTouchLeft = m_VR->get_action_handle("/actions/default/in/BButtonTouchLeft");
            UEVR_ActionHandle LeftShift  = m_VR->get_action_handle("/actions/default/in/ThumbrestTouchLeft");
            UEVR_ActionHandle RightShift = m_VR->get_action_handle("/actions/default/in/ThumbrestTouchRight");
#if 0
            UEVR_ActionHandle RightGrip  = m_VR->get_action_handle("/actions/default/in/BButtonTouchRight");

            // First, we will try to see if we are using a gamepad. If start or select is active and the 
            // openxr read for this is not, we assume gamepad mode and return.
            if(state->Gamepad.wButtons & (XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK))
            {
                // This catches the case where we just set the controllers down and picked up the gamepad. The controllers are still
                // active but we're not using them so we want to return and not do anything.
                if (!m_VR->is_action_active(BTouchLeft, LeftController) && !m_VR->is_action_active(ATouchLeft, LeftController))
                {
                    return;
                }
               
            }
           
            // This is only here because I have vive controllers. The index controllers right grip is active just holding
            // the controllers. So this whole plugin mechanism will be idle until the right grip is detected once.
            // In the vive version of this plugin, it will be the opposite.
            if(m_IndexIndicatorActive == false && m_VR->is_action_active(RightGrip, RightController)) {
                API::get()->log_info("Index-Haptic: detected grip down, enabling index control.");
                m_IndexIndicatorActive = true;
            }
            
            if(m_IndexIndicatorActive == false) return;
#endif
            
            // clear all key down events from last pass
            if(F1Down == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_A)) {
                send_key(VK_F1, KEYUP);
                F1Down = false;
            }
            
            if(F2Down == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_X)) {
                send_key(VK_F2, KEYUP);
                F2Down = false;
            }
            
            if(F3Down == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)) {
                send_key(VK_F3, KEYUP);
                F3Down = false;
            }

            if(F5Down == true && (state->Gamepad.sThumbRY > -5000)) {
                send_key(VK_F5, KEYUP);
                F5Down = false;
            }

            if(F4Down == true && (state->Gamepad.sThumbRY < 5000)) {
                send_key(VK_F4, KEYUP);
                F4Down = false;
            }

            if(F6Down == true && (state->Gamepad.sThumbRX > -5000)) {
                send_key(VK_F6, KEYUP);
                F6Down = false;
            }

            if(F7Down == true && (state->Gamepad.sThumbRX < 5000)) {
                send_key(VK_F7, KEYUP);
                F7Down = false;
            }

            if(LTDown == true && state->Gamepad.bLeftTrigger < 200) {
                LTDown = false;
            }
            
            if(RBShiftDown == true && !(state->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
                RBShiftDown = false;
            }
            
            // Dpad shift is added here since not working in UEVR for some reason.
            if((m_VR->is_action_active(RightShift, RightController) &&
               !(state->Gamepad.wButtons & (XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_Y)))) {
                
                // Left stick down
                if(state->Gamepad.sThumbLY <= -25000) {
                    state->Gamepad.sThumbLY = 0;
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_DPAD_DOWN);
                }

                // Left stick up
                if(state->Gamepad.sThumbLY >= 25000) {
                    state->Gamepad.sThumbLY = 0;
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_DPAD_UP);
                }

                // Left stick left
                if(state->Gamepad.sThumbLX <= -25000) {
                    state->Gamepad.sThumbLX = 0;
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_DPAD_LEFT);
                }

                // Left stick right
                if(state->Gamepad.sThumbLX >= 25000) {
                    state->Gamepad.sThumbLX = 0;
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_DPAD_RIGHT);
                }
               
            }
            
            // Check for left shifting. This is when left trackpad touched but not start or back.
            if(m_VR->is_action_active(LeftShift, LeftController) &&
               !m_VR->is_action_active(ATouchLeft, LeftController) &&
               !m_VR->is_action_active(BTouchLeft, LeftController)) {
                LeftShifting = true;
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_A && F1Down == false) {
                    F1Down = true;
                    send_key(VK_F1, KEYDOWN);
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_A);
                }
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_X && F2Down == false) {
                    F2Down = true;
                    send_key(VK_F2, KEYDOWN);
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_X);
                }
                
                if(state->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB && F3Down == false) {
                    F3Down = true;
                    send_key(VK_F3, KEYDOWN);
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_RIGHT_THUMB);
                }
                
                // Right stick down
                if(state->Gamepad.sThumbRY <= -25000 && F5Down == false) {
                    F5Down = true;
                    send_key(VK_F5, KEYDOWN);
                    state->Gamepad.sThumbRY = 0;
                }

                // Right stick up
                if(state->Gamepad.sThumbRY >= 25000 && F4Down == false) {
                    F4Down = true;
                    send_key(VK_F4, KEYDOWN);
                    state->Gamepad.sThumbRY = 0;
                }

                // Right stick left
                if(state->Gamepad.sThumbRX <= -25000 && F6Down == false) {
                    F6Down = true;
                    send_key(VK_F6, KEYDOWN);
                    state->Gamepad.sThumbRX = 0;
                }

                // Right stick right
                if(state->Gamepad.sThumbRX >= 25000 && F7Down == false) {
                    F7Down = true;
                    send_key(VK_F7, KEYDOWN);
                    state->Gamepad.sThumbRX = 0;
                }
                
                // Left trigger - this is a switch LT, RB flag.
                if(state->Gamepad.bLeftTrigger >= 200 && LTDown == false) {
                    LTDown = true;
                    SwapLtRb = !SwapLtRb;
                    state->Gamepad.bLeftTrigger = 0;
                    m_VR->trigger_haptic_vibration(0.0f, 0.5f, 1.0f, 1000.0f, LeftController);	
                    m_VR->trigger_haptic_vibration(0.0f, 0.5f, 1.0f, 1000.0f, RightController);	
                }
                
            }
            
            if(SwapLtRb == true) {
                bool LT = (state->Gamepad.bLeftTrigger >= 200) ? true : false;
                bool RB = (state->Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? true : false;
                
                state->Gamepad.bLeftTrigger = (RB == true) ? 255 : 0;
                if(LT == true) {
                    state->Gamepad.wButtons |= (XINPUT_GAMEPAD_RIGHT_SHOULDER);
                } else {
                    state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_RIGHT_SHOULDER);
                }
            }
            
            // Clear xinput for start & select / menu & back.
            state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK);
            
            // First, let's try to differentiate start & select buttons based on if the systembutton comes from bottom or top of pad
            if(m_VR->is_action_active_any_joystick(ATouchLeft))
            {
                state->Gamepad.wButtons |= XINPUT_GAMEPAD_START;
                
                // Trigger quick haptic when this is registered the first time.
                if(StartDown == false)
                {
                    StartDown = true;
                    m_VR->trigger_haptic_vibration(0.0f, 0.05f, 1.0f, 1000.0f, LeftController);	
                }
            }
            else
            {
                StartDown = false;
            }
            
            if (m_VR->is_action_active_any_joystick(BTouchLeft))
            {
                state->Gamepad.wButtons |= XINPUT_GAMEPAD_BACK;
                // Trigger quick haptic when this is registered the first time.
                if(SelectDown == false)
                {
                    SelectDown = true;
                    m_VR->trigger_haptic_vibration(0.0f, 0.05f, 1.0f, 1000.0f, LeftController);	
                }
            }
            else
            {
                SelectDown = false;
            }
            
            // Trigger super short haptic on left grip
            //if(state->Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
            if (m_VR->is_action_active(GripButton, LeftController))
            {
                if(LeftGripDown == false)
                {
                    LeftGripDown = true;
                    m_VR->trigger_haptic_vibration(0.0f, 0.05f, 1.0f, 1000.0f, LeftController);					
                }
            }
            else
            {
                if(LeftGripDown == true)
                {
                    LeftGripDown = false;
                    m_VR->trigger_haptic_vibration(0.0f, 0.05f, 1.0f, 1000.0f, LeftController);					
                }
            }
                        
            // Trigger super short haptic on right grip
            if (m_VR->is_action_active(GripButton, RightController))
            {
                if(RightGripDown == false)
                {
                    RightGripDown = true;
                    m_VR->trigger_haptic_vibration(0.0f, 0.05f, 1.0f, 1000.0f, RightController);					
                }
            }
            else
            {
                if(RightGripDown == true)
                {
                    RightGripDown = false;
                    m_VR->trigger_haptic_vibration(0.0f, 0.05f, 1.0f, 1000.0f, RightController);					
                }
            }
		}
    }

};
// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called in some way.
std::unique_ptr<RemapPlugin> g_plugin{new RemapPlugin()};

void DebugPrint(char* Format, ...)
{
  char FormattedMessage[512];    
  va_list ArgPtr = NULL;  
  
  /* Generate the formatted debug message. */        
  va_start(ArgPtr, Format);        
  vsprintf(FormattedMessage, Format, ArgPtr);        
  va_end(ArgPtr); 

  OutputDebugString(FormattedMessage);
}

