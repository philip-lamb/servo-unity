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
	m_buf(NULL),
	m_format(ServoUnityTextureFormat_RGBA32), // Servo's default.
	m_pixelIntFormatGL(0),
	m_pixelFormatGL(0),
	m_pixelTypeGL(0),
	m_pixelSize(0)
{
}

ServoUnityWindowGL::~ServoUnityWindowGL() {
	if (m_buf) {
		free(m_buf);
		m_buf = NULL;
	}
}

bool ServoUnityWindowGL::init(PFN_WINDOWCREATEDCALLBACK windowCreatedCallback, PFN_WINDOWRESIZEDCALLBACK windowResizedCallback, PFN_BROWSEREVENTCALLBACK browserEventCallback)
{
    if (!ServoUnityWindow::init(windowCreatedCallback, windowResizedCallback, browserEventCallback)) return false;
    
	switch (m_format) {
		case ServoUnityTextureFormat_RGBA32:
			m_pixelIntFormatGL = GL_RGBA;
			m_pixelFormatGL = GL_RGBA;
			m_pixelTypeGL = GL_UNSIGNED_BYTE;
			m_pixelSize = 4;
			break;
		case ServoUnityTextureFormat_BGRA32:
			m_pixelIntFormatGL = GL_RGBA;
			m_pixelFormatGL = GL_BGRA;
			m_pixelTypeGL = GL_UNSIGNED_BYTE;
			m_pixelSize = 4;
			break;
		case ServoUnityTextureFormat_ARGB32:
			m_pixelIntFormatGL = GL_RGBA;
			m_pixelFormatGL = GL_BGRA;
			m_pixelTypeGL = GL_UNSIGNED_INT_8_8_8_8; // GL_UNSIGNED_INT_8_8_8_8_REV on big-endian.
			m_pixelSize = 4;
			break;
			//case ServoUnityTextureFormat_ABGR32: // Needs GL_EXT_abgr
			//	m_pixelIntFormatGL = GL_RGBA;
			//	m_pixelFormatGL = GL_ABGR_EXT;
			//	m_pixelTypeGL = GL_UNSIGNED_BYTE;
			//	m_pixelSize = 4;
			//	break;
		case ServoUnityTextureFormat_RGB24:
			m_pixelIntFormatGL = GL_RGB;
			m_pixelFormatGL = GL_RGB;
			m_pixelTypeGL = GL_UNSIGNED_BYTE;
			m_pixelSize = 3;
			break;
		case ServoUnityTextureFormat_BGR24:
			m_pixelIntFormatGL = GL_RGBA;
			m_pixelFormatGL = GL_BGR;
			m_pixelTypeGL = GL_UNSIGNED_BYTE;
			m_pixelSize = 3;
			break;
		case ServoUnityTextureFormat_RGBA4444:
			m_pixelIntFormatGL = GL_RGBA;
			m_pixelFormatGL = GL_RGBA;
			m_pixelTypeGL = GL_UNSIGNED_SHORT_4_4_4_4;
			m_pixelSize = 2;
			break;
		case ServoUnityTextureFormat_RGBA5551:
			m_pixelIntFormatGL = GL_RGBA;
			m_pixelFormatGL = GL_RGBA;
			m_pixelTypeGL = GL_UNSIGNED_SHORT_5_5_5_1;
			m_pixelSize = 2;
			break;
		case ServoUnityTextureFormat_RGB565:
			m_pixelIntFormatGL = GL_RGB;
			m_pixelFormatGL = GL_RGB;
			m_pixelTypeGL = GL_UNSIGNED_SHORT_5_6_5;
			m_pixelSize = 2;
			break;
		default:
			break;
	}

    m_buf = (uint8_t *)calloc(1, m_size.w * m_size.h * m_pixelSize);

	if (m_windowCreatedCallback) (*m_windowCreatedCallback)(m_uidExt, m_uid, m_size.w, m_size.h, m_format);

	return true;
}

ServoUnityWindow::Size ServoUnityWindowGL::size() {
	return m_size;
}

void ServoUnityWindowGL::setSize(ServoUnityWindow::Size size) {
	m_size = size;
	if (m_buf) free(m_buf);
	m_buf = (uint8_t *)calloc(1, m_size.w * m_size.h * m_pixelSize);

    if (m_windowResizedCallback) (*m_windowResizedCallback)(m_uidExt, m_size.w, m_size.h);
}

void ServoUnityWindowGL::setNativePtr(void* texPtr) {
	m_texID = (uint32_t)((uintptr_t)texPtr); // Truncation to 32-bits is the desired behaviour.
}

void* ServoUnityWindowGL::nativePtr() {
	return (void *)((uintptr_t)m_texID); // Extension to pointer-length (usually 64 bits) is the desired behaviour.
}

void ServoUnityWindowGL::initRenderer(CInitOptions cio, void (*wakeup)(void), CHostCallbacks chc) {
    // init_with_gl will capture the active GL context for later use by fill_gl_texture.
    // This will be the Unity GL context.
    init_with_gl(cio, wakeup, chc);
}

void ServoUnityWindowGL::requestUpdate(float timeDelta) {
    SERVOUNITYLOGd("ServoUnityWindowGL::requestUpdate(%f)\n", timeDelta);

    ServoUnityWindow::requestUpdate(timeDelta);
        
    // fill_gl_texture sets the GL context to the same Unity GL context.
    fill_gl_texture(m_texID, m_size.w, m_size.h);
}

#endif // SUPPORT_OPENGL_CORE
