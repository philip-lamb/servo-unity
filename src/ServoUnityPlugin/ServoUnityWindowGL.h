//
// ServoUnityWindowGL.h
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0.If a copy of the MPL was not distributed with this
// file, You can obtain one at https ://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla, Inc.
//
// Author(s): Philip Lamb
//
// An implementation for a Servo window that renders to an OpenGL texture.
//

#pragma once
#include "ServoUnityWindow.h"
#if SUPPORT_OPENGL_CORE

class ServoUnityWindowGL : public ServoUnityWindow
{
private:
	Size m_size;
	uint32_t m_texID;
	int m_format;

public:
	static void initDevice();
	static void finalizeDevice();
	ServoUnityWindowGL(int uid, int uidExt, Size size);
	~ServoUnityWindowGL() ;

	bool init(PFN_WINDOWCREATEDCALLBACK windowCreatedCallback, PFN_WINDOWRESIZEDCALLBACK windowResizedCallback, PFN_BROWSEREVENTCALLBACK browserEventCallback) override;
    RendererAPI rendererAPI() override {return RendererAPI::OpenGLCore;}
	Size size() override;
	void setSize(Size size) override;
	int format() override { return m_format; }
	void setNativePtr(void* texPtr) override;
	void* nativePtr() override;

	void requestUpdate(float timeDelta) override;
	void initRenderer(CInitOptions cio, void (*wakeup)(void), CHostCallbacks chc) override;
};

#endif // SUPPORT_OPENGL_CORE
