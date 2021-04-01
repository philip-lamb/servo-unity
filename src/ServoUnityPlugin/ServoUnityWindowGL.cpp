//
// ServoUnityWindowGL.cpp
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0.If a copy of the MPL was not distributed with this
// file, You can obtain one at https ://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla, Inc.
//
// Author(s): Philip Lamb
//

#include "ServoUnityWindowGL.h"
#if SUPPORT_OPENGL_CORE

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#  include <OpenGL/CGLCurrent.h>
#elif defined(_WIN32)
#  include <gl3w/gl3w.h>
#else 
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glcorearb.h>
#endif
#include <stdlib.h>
#include "servo_unity_internal.h"
#include "servo_unity_log.h"
#include "utils.h"


void ServoUnityWindowGL::initDevice() {
#ifdef _WIN32
	gl3wInit();
#endif
}

void ServoUnityWindowGL::finalizeDevice() {
}

ServoUnityWindowGL::ServoUnityWindowGL(int uid, int uidExt, Size size) :
	ServoUnityWindow(uid, uidExt),
	m_size(size),
	m_texID(0),
	m_format(ServoUnityTextureFormat_RGBA32) // Servo's default.
{
}

ServoUnityWindowGL::~ServoUnityWindowGL() {
}

bool ServoUnityWindowGL::init(PFN_WINDOWCREATEDCALLBACK windowCreatedCallback, PFN_WINDOWRESIZEDCALLBACK windowResizedCallback, PFN_BROWSEREVENTCALLBACK browserEventCallback, const std::string& userAgent)
{
    if (!ServoUnityWindow::init(windowCreatedCallback, windowResizedCallback, browserEventCallback, userAgent)) return false;
    
	if (m_windowCreatedCallback) (*m_windowCreatedCallback)(m_uidExt, m_uid, m_size.w, m_size.h, m_format);

	return true;
}

ServoUnityWindow::Size ServoUnityWindowGL::size() {
	return m_size;
}

void ServoUnityWindowGL::setSize(ServoUnityWindow::Size size) {
	m_size = size;

    if (m_windowResizedCallback) (*m_windowResizedCallback)(m_uidExt, m_size.w, m_size.h);
}

void ServoUnityWindowGL::setNativePtr(void* texPtr) {
	m_texID = (uint32_t)((uintptr_t)texPtr); // Truncation to 32-bits is the desired behaviour.
}

void* ServoUnityWindowGL::nativePtr() {
	return (void *)((uintptr_t)m_texID); // Extension to pointer-length (usually 64 bits) is the desired behaviour.
}

bool ServoUnityWindowGL::initRenderer(CInitOptions cio, void (*wakeup)(void), CHostCallbacks chc) {
    // init_with_gl will capture the active GL context for later use by fill_gl_texture.
    // This will be the Unity GL context.
    init_with_gl(cio, wakeup, chc);
	return true;
}

void ServoUnityWindowGL::requestUpdate(float timeDelta) {
    SERVOUNITYLOGd("ServoUnityWindowGL::requestUpdate(%f)\n", timeDelta);

    ServoUnityWindow::requestUpdate(timeDelta);
        
    // fill_gl_texture sets the GL context to the same Unity GL context.
    fill_gl_texture(m_texID, m_size.w, m_size.h);
}

#endif // SUPPORT_OPENGL_CORE
