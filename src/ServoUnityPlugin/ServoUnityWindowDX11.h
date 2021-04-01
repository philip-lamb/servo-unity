//
// ServoUnityWindowDX11.h
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0.If a copy of the MPL was not distributed with this
// file, You can obtain one at https ://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla, Inc.
//
// Author(s): Philip Lamb
//
// An implementation for a Servo window that renders to a DirectX 11 texture handle.
// Note that this is presently UNIMPLEMENTED, but the class is retained here for
// possible future development.
//

#pragma once
#include "ServoUnityWindow.h"
#if SUPPORT_D3D11
#include <cstdint>
#include <string>
#include <d3d11.h>
#include "IUnityInterface.h"
#include "OpenGLES.h"

class ServoUnityWindowDX11 : public ServoUnityWindow
{
private:

	OpenGLES m_GLES;
	ID3D11Texture2D* m_servoTexPtr;
	Size m_size;
	DXGI_FORMAT m_formatDX;
	int m_format;
	EGLSurface m_EGLSurface;
	GLuint m_texID;
	void *m_unityTexPtr;

public:
	static void initDevice(IUnityInterfaces* unityInterfaces);
	static void finalizeDevice();

	ServoUnityWindowDX11(int uid, int uidExt, Size size);
	~ServoUnityWindowDX11() ;
    //ServoUnityWindowDX11(const ServoUnityWindowDX11&) = delete;
	//void operator=(const ServoUnityWindowDX11&) = delete;

	bool init(PFN_WINDOWCREATEDCALLBACK windowCreatedCallback, PFN_WINDOWRESIZEDCALLBACK windowResizedCallback, PFN_BROWSEREVENTCALLBACK browserEventCallback, const std::string& userAgent) override;
    RendererAPI rendererAPI() override {return RendererAPI::DirectX11;}
	Size size() override;
	void setSize(Size size) override;
	int format() override { return m_format; }
	void setNativePtr(void* texPtr) override;
	void* nativePtr() override;

	void requestUpdate(float timeDelta) override;
    bool initRenderer(CInitOptions cio, void (*wakeup)(void), CHostCallbacks chc) override;
	void cleanupRenderer() override;
};

#endif // SUPPORT_D3D11
