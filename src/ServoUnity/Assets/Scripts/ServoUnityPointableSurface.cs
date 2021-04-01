// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla.
// Copyright (c) 2021 Philip Lamb.
//
// Author(s): Philip Lamb, Patrick O'Shaughnessey

using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ServoUnityPointableSurface : MonoBehaviour
{
    public ServoUnityPlugin servo_unity_plugin = null; // Reference to the plugin. Will be set/cleared by ServoUnityController.
    protected Vector2Int videoSize;
    protected int _windowIndex = 0;

    public void PointerEnter()
    {
        Debug.Log("PointerEnter()");
        servo_unity_plugin?.ServoUnityWindowPointerEvent(_windowIndex, ServoUnityPlugin.ServoUnityPointerEventID.Enter, -1, -1, - 1, -1);
    }

    public void PointerExit()
    {
        Debug.Log("PointerExit()");
        servo_unity_plugin?.ServoUnityWindowPointerEvent(_windowIndex, ServoUnityPlugin.ServoUnityPointerEventID.Exit, -1, -1, -1, -1);
    }

    private Vector2Int GetWindowCoordForTexCoord(Vector2 texCoord)
    {
        return new Vector2Int((int)(texCoord.x * videoSize.x), (int)((1.0f - texCoord.y) * videoSize.y)); // Window origin is top-left.
    }

    public void PointerOver(Vector2 texCoord)
    {
        Vector2Int windowCoord = GetWindowCoordForTexCoord(texCoord);
        Debug.Log("PointerOver(" + windowCoord.x + ", " + windowCoord.y + ")");
        servo_unity_plugin?.ServoUnityWindowPointerEvent(_windowIndex, ServoUnityPlugin.ServoUnityPointerEventID.Over, -1, -1, windowCoord.x, windowCoord.y);
    }

    public void PointerPress(ServoUnityPlugin.ServoUnityPointerEventMouseButtonID button, Vector2 texCoord)
    {
        Vector2Int windowCoord = GetWindowCoordForTexCoord(texCoord);
        Debug.Log("PointerPress(" + windowCoord.x + ", " + windowCoord.y + ")");
        servo_unity_plugin?.ServoUnityWindowPointerEvent(_windowIndex, ServoUnityPlugin.ServoUnityPointerEventID.Press, (int)button, -1, windowCoord.x, windowCoord.y);
    }

    public void PointerRelease(ServoUnityPlugin.ServoUnityPointerEventMouseButtonID button, Vector2 texCoord)
    {
        Vector2Int windowCoord = GetWindowCoordForTexCoord(texCoord);
        Debug.Log("PointerRelease(" + windowCoord.x + ", " + windowCoord.y + ")");
        servo_unity_plugin?.ServoUnityWindowPointerEvent(_windowIndex, ServoUnityPlugin.ServoUnityPointerEventID.Release, (int)button, -1, windowCoord.x, windowCoord.y);
    }

    public void PointerClick(ServoUnityPlugin.ServoUnityPointerEventMouseButtonID button, Vector2 texCoord)
    {
        Vector2Int windowCoord = GetWindowCoordForTexCoord(texCoord);
        Debug.Log("PointerClick(" + windowCoord.x + ", " + windowCoord.y + ")");
        servo_unity_plugin?.ServoUnityWindowPointerEvent(_windowIndex, ServoUnityPlugin.ServoUnityPointerEventID.Click, (int)button, -1, windowCoord.x, windowCoord.y);
    }

    public void PointerScrollDiscrete(Vector2 delta, Vector2 texCoord)
    {
        int scroll_x = (int)delta.x;
        int scroll_y = (int)delta.y;
        Vector2Int windowCoord = GetWindowCoordForTexCoord(texCoord);
        Debug.Log("PointerScrollDiscrete(" + scroll_x + ", " + scroll_y + ", " + windowCoord.x + ", " + windowCoord.y + ")");
        servo_unity_plugin?.ServoUnityWindowPointerEvent(_windowIndex, ServoUnityPlugin.ServoUnityPointerEventID.ScrollDiscrete, scroll_x, scroll_y, windowCoord.x, windowCoord.y);
    }
}
