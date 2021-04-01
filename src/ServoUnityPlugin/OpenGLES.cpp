// Copyright (C) 2002-2013 The ANGLE Project Authors. 
// Portions Copyright (C) Microsoft Corporation.
// Portions Copyright (C) Kazendi, Ltd.
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

#include "OpenGLES.h"
#include "servo_unity_log.h"
#include <wrl/client.h> // ComPtr

OpenGLES::OpenGLES() :
    mEglConfig(nullptr), mEglDisplay(EGL_NO_DISPLAY),
    mEglContext(EGL_NO_CONTEXT)
{
    SERVOUNITYLOGd("OpenGLES::OpenGLES()");
}

OpenGLES::~OpenGLES() { Cleanup(); }

bool OpenGLES::Initialize() {
    const EGLint contextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };

    // Based on Angle MS template.
    const EGLint defaultDisplayAttributes[] = {
        // These are the default display attributes, used to request ANGLE's D3D11
        // renderer.
        // eglInitialize will only succeed with these attributes if the hardware
        // supports D3D11 Feature Level 10_0+.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,

        // EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE is an optimization that
        // can have large performance benefits on mobile devices.
        EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE, EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE, // alternative: EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE

        // EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE is an option that
        // enables ANGLE to automatically call
        // the IDXGIDevice3::Trim method on behalf of the application when it gets
        // suspended.
        // Calling IDXGIDevice3::Trim when an application is suspended is a
        // Windows Store application certification
        // requirement.
        EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
        EGL_NONE,
    };

    // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay.
    // It allows us to pass in display attributes, used to configure D3D11.
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (!eglGetPlatformDisplayEXT) {
        SERVOUNITYLOGe("OpenGLES::Initialize: Failed to get function eglGetPlatformDisplayEXT.\n");
        return false;
    }

    //
    // To initialize the display, we make three sets of calls to
    // eglGetPlatformDisplayEXT and eglInitialize, with varying parameters passed
    // to eglGetPlatformDisplayEXT: 1) The first calls uses
    // "defaultDisplayAttributes" as a parameter. This corresponds to D3D11
    // Feature Level 10_0+. 2) If eglInitialize fails for step 1 (e.g. because
    // 10_0+ isn't supported by the default GPU), then we try again
    //    using "fl9_3DisplayAttributes". This corresponds to D3D11 Feature Level
    //    9_3.
    // 3) If eglInitialize fails for step 2 (e.g. because 9_3+ isn't supported by
    // the default GPU), then we try again
    //    using "warpDisplayAttributes".  This corresponds to D3D11 Feature Level
    //    11_0 on WARP, a D3D11 software rasterizer.
    //

    // This tries to initialize EGL to D3D11 Feature Level 10_0+. See above
    // comment for details.
    mEglDisplay = eglGetPlatformDisplayEXT( EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, defaultDisplayAttributes);
    if (mEglDisplay == EGL_NO_DISPLAY) {
        SERVOUNITYLOGe("OpenGLES::Initialize: Failed to get EGL display.\n");
        return false;
    }

    if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE) {
        // This tries to initialize EGL to D3D11 Feature Level 9_3, if 10_0+ is
        // unavailable (e.g. on some mobile devices).

        const EGLint fl9_3DisplayAttributes[] = {
            // These can be used to request ANGLE's D3D11 renderer, with D3D11 Feature
            // Level 9_3.
            // These attributes are used if the call to eglInitialize fails with the
            // default display attributes.
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 9,
            EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 3,
            EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE, EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE,
            EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
            EGL_NONE,
        };

        mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, fl9_3DisplayAttributes);
        if (mEglDisplay == EGL_NO_DISPLAY) {
            SERVOUNITYLOGe("OpenGLES::Initialize: Failed to get EGL display.\n");
            return false;
        }

        if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE) {
            // This initializes EGL to D3D11 Feature Level 11_0 on WARP, if 9_3+ is
            // unavailable on the default GPU.
 
            const EGLint warpDisplayAttributes[] = {
                // These attributes can be used to request D3D11 WARP.
                // They are used if eglInitialize fails with both the default display
                // attributes and the 9_3 display attributes.
                EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
                EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE,
                EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE, EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE,
                EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
                EGL_NONE,
            };

            mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, warpDisplayAttributes);
            if (mEglDisplay == EGL_NO_DISPLAY) {
                SERVOUNITYLOGe("OpenGLES::Initialize: Failed to get EGL display.\n");
                return false;
            }

            if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE) {
                // If all of the calls to eglInitialize returned EGL_FALSE then an error
                // has occurred.
                SERVOUNITYLOGe("OpenGLES::Initialize: Failed to initialize EGL.\n");
                return false;
            }
        }
    }

    const EGLint configAttributes[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                                    EGL_RED_SIZE, 8,
                                    EGL_GREEN_SIZE,  8,
                                    EGL_BLUE_SIZE, 8,
                                    EGL_ALPHA_SIZE, 8,
                                    //EGL_DEPTH_SIZE, 24, // Don't need depth buffer for surfman output.
                                    //EGL_STENCIL_SIZE, 8, // Don't need stencil buffer for surfman output.
                                    EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
                                    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                                    EGL_NONE,
    };
    EGLint numConfigs = 0;
    if ((eglChooseConfig(mEglDisplay, configAttributes, &mEglConfig, 1, &numConfigs) == EGL_FALSE) || (numConfigs == 0)) {
        SERVOUNITYLOGe("OpenGLES::Initialize: Failed to choose first EGLConfig.\n");
        return false;
    }

    mEglContext = eglCreateContext(mEglDisplay, mEglConfig, EGL_NO_CONTEXT, contextAttributes);
    if (mEglContext == EGL_NO_CONTEXT) {
        SERVOUNITYLOGe("OpenGLES::Initialize: Failed to create EGL context.\n");
        return false;
    }

    return true;
}

void OpenGLES::Cleanup() {
    if (mEglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (mEglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mEglDisplay, mEglContext);
            mEglContext = EGL_NO_CONTEXT;
        }
        eglTerminate(mEglDisplay);
        mEglDisplay = EGL_NO_DISPLAY;
    }
}

void OpenGLES::Reset() {
    Cleanup();
    Initialize();
}

EGLSurface OpenGLES::CreateSurface(ID3D11Texture2D* tex2D)
{
    if (!tex2D) {
        SERVOUNITYLOGe("OpenGLES::CreateSurface: no texture supplied.\n");
        return EGL_NO_SURFACE;
    }

    D3D11_TEXTURE2D_DESC texDesc = { 0 };
    tex2D->GetDesc(&texDesc);
    //SERVOUNITYLOGd("OpenGLES::CreateSurface: Texture is %dx%d, DXGI_FORMAT=%d, MipLevels=%d, D3D11_USAGE Usage=%d, BindFlags=%d, CPUAccessFlags=%d, MiscFlags=%d\n", texDesc.Width, texDesc.Height, texDesc.Format, texDesc.MipLevels, texDesc.Usage, texDesc.BindFlags, texDesc.CPUAccessFlags, texDesc.MiscFlags);
    
    // Perform a few essential checks on supplied texture.
    if (!texDesc.Width || !texDesc.Height) {
        SERVOUNITYLOGe("OpenGLES::CreateSurface: texture has 0 width or height.\n");
        return EGL_NO_SURFACE;
    }
    if (!(texDesc.MiscFlags & (D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX))) {
        SERVOUNITYLOGe("OpenGLES::CreateSurface: texture must be shared, but missing flag D3D11_RESOURCE_MISC_SHARED or D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX.\n");
        return EGL_NO_SURFACE;
    }

    IDXGIResource* pDXGIResource;
    HRESULT hr = tex2D->QueryInterface(__uuidof(IDXGIResource), (void**)&pDXGIResource);
    if FAILED(hr) {
        SERVOUNITYLOGe("OpenGLES::CreateSurface: can't get DXGI resource from texture.\n");
        return EGL_NO_SURFACE;
    }

    HANDLE sharedHandle;
    hr = pDXGIResource->GetSharedHandle(&sharedHandle);
    if FAILED(hr) {
        SERVOUNITYLOGe("OpenGLES::CreateSurface: can't get shared handle from texture. Was it created with usage D3D11_RESOURCE_MISC_SHARED or D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX?\n");
        return EGL_NO_SURFACE;
    }

    EGLint pBufferAttributes[] = {
        EGL_WIDTH, (EGLint)texDesc.Width,
        EGL_HEIGHT, (EGLint)texDesc.Height,
        EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
        EGL_NONE
    };

    EGLSurface surface = eglCreatePbufferFromClientBuffer(mEglDisplay, EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE, sharedHandle, mEglConfig, pBufferAttributes);
    if (surface == EGL_NO_SURFACE)  {
        SERVOUNITYLOGe("OpenGLES::CreateSurface: can't create surface.\n");
    }

    return surface;
}

void OpenGLES::GetSurfaceDimensions(const EGLSurface surface, EGLint* width, EGLint* height)
{
    eglQuerySurface(mEglDisplay, surface, EGL_WIDTH, width);
    eglQuerySurface(mEglDisplay, surface, EGL_HEIGHT, height);
}

void OpenGLES::DestroySurface(const EGLSurface surface)
{
    if (mEglDisplay != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE) {
        eglDestroySurface(mEglDisplay, surface);
    }
}

void OpenGLES::MakeCurrent(const EGLSurface surface)
{
    if (eglMakeCurrent(mEglDisplay, surface, surface, mEglContext) == EGL_FALSE) {
        SERVOUNITYLOGe("Failed to make EGL surface current");
    }
}

EGLBoolean OpenGLES::SwapBuffers(const EGLSurface surface)
{
    return (eglSwapBuffers(mEglDisplay, surface));
}

GLuint OpenGLES::CreateSurfaceTexture(const EGLSurface surface)
{
    GLuint glTex;
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);
    if (!eglBindTexImage(mEglDisplay, surface, EGL_BACK_BUFFER)) { // https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglBindTexImage.xhtml
        SERVOUNITYLOGe("Unable to bind surface to texture.");
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &glTex);
        return 0;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return glTex;
}

void OpenGLES::DestroySurfaceTexture(const GLuint texID, const EGLSurface surface)
{
    eglReleaseTexImage(mEglDisplay, surface, EGL_BACK_BUFFER);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &texID);
}
