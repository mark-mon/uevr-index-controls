#define  _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory>

#include "uevr/Plugin.hpp"

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
    RemapPlugin() = default;
    
    void on_dllmain(HANDLE handle) override {
    }

    void on_initialize() override {
      OutputDebugString("Initializing Index-Haptic\n");
         
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

		if (m_OpenXr == true)
		{
            UEVR_ActionHandle GripButton = m_VR->get_action_handle("/actions/default/in/Grip");
            UEVR_ActionHandle SystemButton = m_VR->get_action_handle("/actions/default/in/SystemButton");
            
            // Clear xinput for start & select / menu & back.
            state->Gamepad.wButtons &= ~(XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK);
            
            // First, let's try to differentiate start & select buttons based on if the systembutton comes from left or right controller
            if (m_VR->is_action_active(SystemButton, RightController))
            {
                state->Gamepad.wButtons |= XINPUT_GAMEPAD_START;
                
                // Trigger quick haptic when this is registered the first time.
                if(StartDown == false)
                {
                    StartDown = true;
                    m_VR->trigger_haptic_vibration(0.0f, 0.05f, 1.0f, 1000.0f, RightController);	
                }
            }
            else
            {
                StartDown = false;
            }
            
            if (m_VR->is_action_active(SystemButton, LeftController))
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
                LeftGripDown = false;
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
                RightGripDown = false;
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

