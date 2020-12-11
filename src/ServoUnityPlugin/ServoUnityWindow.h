//
// ServoUnityWindow.h
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0.If a copy of the MPL was not distributed with this
// file, You can obtain one at https ://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla, Inc.
//
// Author(s): Philip Lamb
//

#pragma once

#include "servo_unity_c.h"
#include "simpleservo2.h"
#include <string>
#include <cstdint>
#include <string>
#include <deque>
#include <functional>
#include <mutex>

class ServoUnityWindow
{
protected:
	ServoUnityWindow(int uid, int uidExt);
	
	int m_uid;
	int m_uidExt;
    PFN_WINDOWCREATEDCALLBACK m_windowCreatedCallback;
    PFN_WINDOWRESIZEDCALLBACK m_windowResizedCallback;
    PFN_BROWSEREVENTCALLBACK m_browserEventCallback;

	virtual void initRenderer(CInitOptions opts, void (*wakeup)(void), CHostCallbacks callbacks) = 0;

private:
	static void on_load_started(void);
    static void on_load_ended(void);
    static void on_title_changed(const char *title);
    static bool on_allow_navigation(const char *url);
    static void on_url_changed(const char *url);
    static void on_history_changed(bool can_go_back, bool can_go_forward);
    static void on_animating_changed(bool animating);
    static void on_shutdown_complete(void);
    static void on_ime_show(const char *text, int32_t x, int32_t y, int32_t width, int32_t height);
    static void on_ime_hide(void);
    static const char *get_clipboard_contents(void);
    static void set_clipboard_contents(const char *contents);
    static void on_media_session_metadata(const char *title, const char *album, const char *artist);
    static void on_media_session_playback_state_change(CMediaSessionPlaybackState state);
    static void on_media_session_set_position_state(double duration, double position, double playback_rate);
    static void prompt_alert(const char *message, bool trusted);
    static CPromptResult prompt_ok_cancel(const char *message, bool trusted);
    static CPromptResult prompt_yes_no(const char *message, bool trusted);
    static const char *prompt_input(const char *message, const char *def, bool trusted);
    static void on_devtools_started(CDevtoolsServerState result, unsigned int port, const char *token);
    static void show_context_menu(const char *title, const char *const *items_list, uint32_t items_size);
    static void on_log_output(const char *buffer, uint32_t buffer_length);
    static void wakeup(void);

    bool m_updateContinuously;
    bool m_updateOnce;
    std::mutex m_updateLock;
    std::string m_title;
    std::string m_URL;
    std::deque< std::function<void()> > m_servoTasks;
    std::mutex m_servoTasksLock;
    typedef struct {int uidExt; int eventType; int eventData1; int eventData2; } BROWSEREVENTCALLBACKTASK;
    std::deque< BROWSEREVENTCALLBACKTASK > m_browserEventCallbackTasks;
    std::mutex m_browserEventCallbackTasksLock;
    void runOnServoThread(std::function<void()> task);
    void queueBrowserEventCallbackTask(int uidExt, int eventType, int eventData1, int eventData2);
    bool m_waitingForShutdown;

public:
    static ServoUnityWindow *s_servo;

	virtual ~ServoUnityWindow() {};

	enum RendererAPI {
		None = 0,
		Unknown,
		DirectX11,
		OpenGLCore
	};

	enum class BrowserEventType : uint8_t {
		None = 0,
		IME,
		Total
	};

	struct Size {
		int w;
		int h;
	};

	int uid() { return m_uid; }
	int uidExt() { return m_uidExt; }
	void setUidExt(int uidExt) { m_uidExt = uidExt; }
	virtual bool init(PFN_WINDOWCREATEDCALLBACK windowCreatedCallback, PFN_WINDOWRESIZEDCALLBACK windowResizedCallback, PFN_BROWSEREVENTCALLBACK browserEventCallback);
	
	virtual RendererAPI rendererAPI() = 0;
	virtual Size size() = 0;
	virtual void setSize(Size size) = 0;
	virtual int format() = 0;
	virtual void setNativePtr(void* texPtr) = 0;
	virtual void* nativePtr() = 0;
	
	/// Request an update to the window texture. Must be called from render thread.
	virtual void requestUpdate(float timeDelta);
	
    /// Notify that the renderer is going away and should be cleaned up. Must be called from render thread.
    void cleanupRenderer(void);
	
	void CloseServoWindow() {}
	
    void serviceWindowEvents(void);
    std::string windowTitle(void);
    std::string windowURL(void);
    
	void pointerEnter();
	void pointerExit();
	void pointerOver(int x, int y);
	void pointerPress(int button, int x, int y);
	void pointerRelease(int button, int x, int y);
	void pointerClick(int button, int x, int y);
    void pointerScrollDiscrete(int x_scroll, int y_scroll, int x, int y); // x and y are a discrete scroll count, e.g. count of mousewheel "clicks".
	void keyEvent(int upDown, int keyCode, int character);

    void refresh();
    void reload();
    void stop();
    void goBack();
    void goForward();
    void goHome();
    void navigate(const std::string& urlOrSearchString);
};

