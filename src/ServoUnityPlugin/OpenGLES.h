// Copyright (C) 2002-2013 The ANGLE Project Authors. 
// Portions Copyright (C) Microsoft Corporation.
// Portions Copyright (C) Kazendi, Ltd.
// Portions Copyright (C) Eden Networks Ltd.
//
// BSD License
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//     Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//
//     Redistributions in binary form must reproduce the above 
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//
//     Neither the name of TransGaming Inc., Google Inc., 3DLabs Inc.
//     Ltd., Microsoft Corporation, nor the names of their contributors
//     may be used to endorse or promote products derived from this
//     software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once
#include "servo_unity_c.h"
#if SUPPORT_D3D11 // At present, just EGL via ANGLE on DX11, but support for others could be added.

#if defined(_WIN32)
#  if WINDOWS_UWP
#    include <angle_windowsstore.h>
#  else
#    define EGL_EGL_PROTOTYPES 1
#  endif
#endif
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <EGL/eglext_angle.h>
#include <d3d11.h>
#include <GLES2/gl2.h>

class OpenGLES {
public:
    OpenGLES();
    ~OpenGLES();
    bool Initialize();
    EGLSurface CreateSurface(ID3D11Texture2D* tex2D);
    void GetSurfaceDimensions(const EGLSurface surface, EGLint* width, EGLint* height);
    void DestroySurface(EGLSurface* surface);
    void MakeCurrent(const EGLSurface surface);
    EGLBoolean SwapBuffers(const EGLSurface surface);
    GLuint CreateSurfaceTexture(const EGLSurface surface);
    void DestroySurfaceTexture(GLuint* texID, const EGLSurface surface);
    void Reset();
    void Cleanup();

private:
    EGLDisplay mEglDisplay;
    EGLContext mEglContext;
    EGLConfig mEglConfig;
};

#endif // SUPPORT_D3D11
