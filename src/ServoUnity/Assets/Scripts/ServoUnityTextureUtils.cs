// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla.
// Copyright (c) 2020-2021 Kazendi Ltd.
//
// Author(s): Philip Lamb, Patrick O'Shaughnessey


using UnityEngine;

public class ServoUnityTextureUtils : MonoBehaviour
{
    public enum VideoSurfaceColliderType
    {
        None, // No Collider will be added.
        Mesh, // (Default.) Adds a MeshCollider coincident with the surface's mesh.
        Box   // Adds a BoxCollider with depth equal to 5% of the surface's diagonal length, with front face at local z = 0.0f and lower edge at local y = 0.0f.
    }

    public static Texture2D CreateTexture(int width, int height, TextureFormat format)
    {
        // Check parameters.
        if (width <= 0 || height <= 0)
        {
            Debug.LogError("Error: Cannot configure video texture with invalid size: " + width + "x" + height);
            return null;
        }
        
        Texture2D vt = new Texture2D(width, height, format, false);
        vt.hideFlags = HideFlags.HideAndDontSave;
        vt.filterMode = FilterMode.Bilinear;
        vt.wrapMode = TextureWrapMode.Clamp;
        vt.anisoLevel = 0;

        // Initialise the video texture to black.
        Color32[] arr = new Color32[width * height];
        Color32 blackOpaque = new Color32(0, 0, 0, 255);
        for (int i = 0; i < arr.Length; i++) arr[i] = blackOpaque;
        vt.SetPixels32(arr);
        vt.Apply(); // Pushes all SetPixels*() ops to texture.
        arr = null;

        return vt;
    }

    // Creates a GameObject in layer 'layer' which renders a mesh displaying the video stream.
    public static GameObject Create2DVideoSurface(Texture2D vt, float textureScaleU, float textureScaleV, float width,
        float height, int layer, bool flipX, bool flipY, VideoSurfaceColliderType colliderType = VideoSurfaceColliderType.Mesh)
    {
        // Check parameters.
        if (!vt)
        {
            Debug.LogError("Error: CreateWindowMesh null Texture2D");
            return null;
        }

        // Create new GameObject to hold mesh.
        GameObject vmgo = new GameObject("Video source");
        if (vmgo == null)
        {
            Debug.LogError("Error: CreateWindowMesh cannot create GameObject.");
            return null;
        }

        vmgo.layer = layer;

        // Create a material which uses our "TextureNoLight" shader, and paints itself with the texture.
        Shader shaderSource = Shader.Find("TextureAlphaNoLight");
        Material vm = new Material(shaderSource); //servoUnity.Properties.Resources.VideoPlaneShader;
        vm.hideFlags = HideFlags.HideAndDontSave;
        //Debug.Log("Created video material");

        MeshFilter filter = vmgo.AddComponent<MeshFilter>();
        MeshRenderer meshRenderer = vmgo.AddComponent<MeshRenderer>();
        meshRenderer.shadowCastingMode = UnityEngine.Rendering.ShadowCastingMode.Off;
        meshRenderer.receiveShadows = false;
 
        vmgo.GetComponent<Renderer>().material = vm;

        if (colliderType == VideoSurfaceColliderType.Mesh)
        {
            vmgo.AddComponent<MeshCollider>();
        }
        else if (colliderType == VideoSurfaceColliderType.Box)
        {
            vmgo.AddComponent<BoxCollider>();
        }

        Configure2DVideoSurface(vmgo, vt, textureScaleU, textureScaleV, width, height, flipX, flipY);
        return vmgo;
    }

    public static Mesh CreateVideoMesh(float textureScaleU, float textureScaleV, float width, float height, bool flipX, bool flipY)
    {
        // Now create a mesh appropriate for displaying the video, a mesh filter to instantiate that mesh,
        // and a mesh renderer to render the material on the instantiated mesh.
        Mesh m = new Mesh();
        m.Clear();
        m.vertices = new Vector3[]
        {
            new Vector3(-width * 0.5f, 0.0f, 0.0f),
            new Vector3(width * 0.5f, 0.0f, 0.0f),
            new Vector3(width * 0.5f, height, 0.0f),
            new Vector3(-width * 0.5f, height, 0.0f),
        };
        m.normals = new Vector3[]
        {
            new Vector3(0.0f, 0.0f, 1.0f),
            new Vector3(0.0f, 0.0f, 1.0f),
            new Vector3(0.0f, 0.0f, 1.0f),
            new Vector3(0.0f, 0.0f, 1.0f),
        };
        float u1 = flipX ? textureScaleU : 0.0f;
        float u2 = flipX ? 0.0f : textureScaleU;
        float v1 = flipY ? textureScaleV : 0.0f;
        float v2 = flipY ? 0.0f : textureScaleV;
        m.uv = new Vector2[] {
            new Vector2(u1, v1),
            new Vector2(u2, v1),
            new Vector2(u2, v2),
            new Vector2(u1, v2)
        };
        m.triangles = new int[] {
            2, 1, 0,
            3, 2, 0
        };


        return m;
    }

    /// <summary>
    /// Given a GameObject holding a Renderer, MeshFilter, and MeshCollider, create (or replace) the
    /// mesh filter & collider's mesh with a new mesh of appropriate dimensions and texture coordinates
    /// to display the texture (as determined by width, height, textureScaleU, textureScaleV, flipX, and flipY).
    /// </summary>
    /// <param name="vmgo"></param>
    /// <param name="vt"></param>
    /// <param name="textureScaleU"></param>
    /// <param name="textureScaleV"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="flipX"></param>
    /// <param name="flipY"></param>
    public static void Configure2DVideoSurface(GameObject vmgo, Texture2D vt, float textureScaleU, float textureScaleV,
        float width, float height, bool flipX, bool flipY)
    {
        // Check parameters.
        if (!vt) {
            Debug.LogError("Error: CreateWindowMesh null Texture2D");
            return;
        }
        // Create the mesh
        Mesh m = ServoUnityTextureUtils.CreateVideoMesh(textureScaleU, textureScaleV, width, height, flipX, flipY);
        
        // Assign the texture to the window's material
        Material vm = vmgo.GetComponent<Renderer>().material;
        vm.mainTexture = vt;

        // Assign the mesh to the mesh filter
        MeshFilter filter = vmgo.GetComponent<MeshFilter>();
        filter.mesh = m;

        // Update collider, if present.
        Collider c = vmgo.GetComponent<Collider>();
        if (c != null)
        {
            if (c is MeshCollider)
            {
                (c as MeshCollider).sharedMesh = filter.sharedMesh;
            }
            else if (c is BoxCollider)
            {
                float depth = (new Vector2(width, height)).magnitude * 0.05f;
                (c as BoxCollider).size = new Vector3(width, height, depth);
                (c as BoxCollider).center = new Vector3(0.0f, height / 2.0f, depth / 2.0f); // Place box's front face at z = 0.0f, and lower edge at y = 0.0f.
            }
        }
    }

    public static TextureFormat GetTextureFormatFromNativeTextureFormat(int formatNative)
    {
        switch (formatNative)
        {
            case 1: // ServoUnityTextureFormat_RGBA32
                return TextureFormat.RGBA32;
            case 2: // ServoUnityTextureFormat_BGRA32
                return TextureFormat.BGRA32;
            case 3: // ServoUnityTextureFormat_ARGB32
                return TextureFormat.ARGB32;
            //case 4: // ServoUnityTextureFormat_ABGR32
            //    format = TextureFormat.ABGR32;
            case 5: // ServoUnityTextureFormat_RGB24
                return TextureFormat.RGB24;
            //case 6: // ServoUnityTextureFormat_BGR24
            //    format = TextureFormat.BGR24;
            case 7: // ServoUnityTextureFormat_RGBA4444
                return TextureFormat.RGBA4444;
            //case 8: // ServoUnityTextureFormat_RGBA5551
            //    format = TextureFormat.RGBA5551;
            case 9: // ServoUnityTextureFormat_RGB565
                return TextureFormat.RGB565;
            default: // ServoUnityTextureFormat_Invalid
                return (TextureFormat)0;
        }
    }
}