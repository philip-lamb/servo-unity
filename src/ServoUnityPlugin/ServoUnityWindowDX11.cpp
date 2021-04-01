//
// ServoUnityWindowDX11.cpp
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0.If a copy of the MPL was not distributed with this
// file, You can obtain one at https ://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla, Inc.
//
// Author(s): Philip Lamb
//

#include <stdlib.h>
#include "ServoUnityWindowDX11.h"
#if SUPPORT_D3D11
#include "IUnityGraphicsD3D11.h"
#include "servo_unity_log.h"

#include <assert.h>
#include <stdio.h>

static ID3D11Device* s_D3D11Device = nullptr;

void ServoUnityWindowDX11::initDevice(IUnityInterfaces* unityInterfaces) {
	IUnityGraphicsD3D11* ud3d = unityInterfaces->Get<IUnityGraphicsD3D11>();
	s_D3D11Device = ud3d->GetDevice();
}

void ServoUnityWindowDX11::finalizeDevice() {
	s_D3D11Device = nullptr; // The object itself being owned by Unity will go away without our help, but we should clear our weak reference.
}

ServoUnityWindowDX11::ServoUnityWindowDX11(int uid, int uidExt, Size size) :
	ServoUnityWindow(uid, uidExt),
	m_GLES(),
	m_servoTexPtr(nullptr),
	m_size(size),
	m_formatDX(DXGI_FORMAT_UNKNOWN),
	m_format(ServoUnityTextureFormat_Invalid),
	m_EGLSurface(EGL_NO_SURFACE),
	m_texID(0),
    m_unityTexPtr(nullptr)
{
}

ServoUnityWindowDX11::~ServoUnityWindowDX11() {
}

static int getServoUnityTextureFormatForDXGIFormat(DXGI_FORMAT format)
{
	switch (format) {
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return ServoUnityTextureFormat_RGBA32;
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			return ServoUnityTextureFormat_BGRA32;
			break;
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return ServoUnityTextureFormat_RGBA4444;
			break;
		case DXGI_FORMAT_B5G6R5_UNORM:
			return ServoUnityTextureFormat_RGB565;
			break;
		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return ServoUnityTextureFormat_RGBA5551;
			break;
		default:
			return ServoUnityTextureFormat_Invalid;
	}
}

bool ServoUnityWindowDX11::init(PFN_WINDOWCREATEDCALLBACK windowCreatedCallback, PFN_WINDOWRESIZEDCALLBACK windowResizedCallback, PFN_BROWSEREVENTCALLBACK browserEventCallback, const std::string& userAgent)
{
    if (!ServoUnityWindow::init(windowCreatedCallback, windowResizedCallback, browserEventCallback, userAgent)) return false;

	//m_formatDX = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_formatDX = DXGI_FORMAT_B8G8R8A8_UNORM; // https://github.com/microsoft/angle/issues/124#issuecomment-313821471
	m_format = getServoUnityTextureFormatForDXGIFormat(m_formatDX);

	// Actual texture creation must be done lazily on the rendering thread.

	if (m_windowCreatedCallback) (*m_windowCreatedCallback)(m_uidExt, m_uid, m_size.w, m_size.h, m_format);

    return true;
}

ServoUnityWindow::Size ServoUnityWindowDX11::size() {
	return m_size;
}

void ServoUnityWindowDX11::setSize(ServoUnityWindow::Size size) {
	// TODO: request change in the Servo window size.

    if (m_windowResizedCallback) (*m_windowResizedCallback)(m_uidExt, m_size.w, m_size.h);
}

void ServoUnityWindowDX11::setNativePtr(void* texPtr) {
	m_unityTexPtr = texPtr;
}

void* ServoUnityWindowDX11::nativePtr() {
	return m_unityTexPtr;
}

bool ServoUnityWindowDX11::initRenderer(CInitOptions cio, void (*wakeup)(void), CHostCallbacks chc) {

	// Create the texture that will receive buffers from surfman (via ANGLE's DirectX interop).
	D3D11_TEXTURE2D_DESC descServo = { 0 };
	descServo.Width = m_size.w;
	descServo.Height = m_size.h;
	descServo.Format = m_formatDX;
	descServo.MipLevels = 1;
	descServo.ArraySize = 1;
	descServo.SampleDesc.Count = 1;
	descServo.SampleDesc.Quality = 0;
	descServo.Usage = D3D11_USAGE_DEFAULT;
	descServo.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	descServo.CPUAccessFlags = 0;
	descServo.MiscFlags = D3D11_RESOURCE_MISC_SHARED; // D3D11_RESOURCE_MISC_SHARED or D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX.
	HRESULT hr = s_D3D11Device->CreateTexture2D(&descServo, nullptr, &m_servoTexPtr);
	if FAILED(hr) {
		SERVOUNITYLOGe("Error: Unable to create texture.\n");
		return false;
	}

	// Set up EGL context.
	if (!m_GLES.Initialize()) {
		SERVOUNITYLOGe("Unable to initialise EGL.\n");
		return false;
	}
	if ((m_EGLSurface = m_GLES.CreateSurface(m_servoTexPtr)) == EGL_NO_SURFACE) {
		SERVOUNITYLOGe("Unable to create EGL surface.\n");
		return false;
	}
	m_GLES.MakeCurrent(m_EGLSurface);

	m_texID = m_GLES.CreateSurfaceTexture(m_EGLSurface);

    // init_with_egl will capture the active EGL context for later use by fill_gl_texture.
    // This will be the Unity EGL context.
    init_with_egl(cio, wakeup, chc);
	return true;
}

void ServoUnityWindowDX11::cleanupRenderer()
{
	m_GLES.DestroySurfaceTexture(m_texID, m_EGLSurface);
	m_texID = 0;
	m_GLES.DestroySurface(m_EGLSurface);
	m_EGLSurface = EGL_NO_SURFACE;
	m_GLES.Cleanup();
	// TODO: Also clean up DirectX textures.

	ServoUnityWindow::cleanupRenderer();
}

void ServoUnityWindowDX11::requestUpdate(float timeDelta) {
    SERVOUNITYLOGd("ServoUnityWindowDX11::requestUpdate(%f)\n", timeDelta);

    ServoUnityWindow::requestUpdate(timeDelta);

	m_GLES.MakeCurrent(m_EGLSurface);

	if (!fill_gl_texture(m_texID, m_size.w, m_size.h)) {
		SERVOUNITYLOGd("ServoUnityWindowDX11::requestUpdate no buffer pending.\n");
		return;
	}

	// Need to flush here to ensure writes have finished before we use in DirectX.
	glFlush();

	if (!m_servoTexPtr) {
		SERVOUNITYLOGi("ServoUnityWindowDX11::requestUpdate() null m_servoTexPtr.\n");
		return;
	}
	if (!m_unityTexPtr) {
		SERVOUNITYLOGi("ServoUnityWindowDX11::requestUpdate() null m_unityTexPtr.\n");
		return;
	}

	ID3D11DeviceContext* ctx = NULL;
	s_D3D11Device->GetImmediateContext(&ctx);

	D3D11_TEXTURE2D_DESC descUnity = { 0 };
	((ID3D11Texture2D*)m_unityTexPtr)->GetDesc(&descUnity);
	//SERVOUNITYLOGd("Unity texture is %dx%d, DXGI_FORMAT=%d (ServoUnityTextureFormat=%d), MipLevels=%d, D3D11_USAGE Usage=%d, BindFlags=%d, CPUAccessFlags=%d, MiscFlags=%d\n", descUnity.Width, descUnity.Height, descUnity.Format, getServoUnityTextureFormatForDXGIFormat(descUnity.Format), descUnity.MipLevels, descUnity.Usage, descUnity.BindFlags, descUnity.CPUAccessFlags, descUnity.MiscFlags);

	D3D11_TEXTURE2D_DESC descServo = { 0 };
	m_servoTexPtr->GetDesc(&descServo);
	if (descServo.Width != descUnity.Width || descServo.Height != descServo.Height) {
		SERVOUNITYLOGe("Error: Unity texture size %dx%d does not match Servo texture size %dx%d.\n", descUnity.Width, descUnity.Height, descServo.Width, descServo.Height);
	} else {
		ctx->CopyResource((ID3D11Texture2D*)m_unityTexPtr, m_servoTexPtr);
	}

	ctx->Release();
}

#endif // SUPPORT_D3D11
