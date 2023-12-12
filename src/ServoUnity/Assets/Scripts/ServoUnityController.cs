// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
// If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2020 Mozilla.
// Copyright (c) 2020-2021 Kazendi Ltd.
//
// Author(s): Philip Lamb, Thomas Moore, Patrick O'Shaughnessey
//
// ServoUnityController is the core controller in the operation of the servo
// browser. Details of the classes and processes involved in bootstrap are
// described below:
//
//                              
//  ServoUnityController  ServoUnityWindow           Unity Plugin             libsimpleservo2 
//       +                  +                        servo_unity.dll)                |                   
//       +                  +                            +                           |   
//   [Start]                +                            +                           |
//       |                  +     servoUnityInit         +                           |   
//       | --------------------------------------------> |                           |
//       +                  +                      Initialize plugin,                |
//       +                  +                      recording passed-in               |
//       +                  +                      callbacks                         |      
//       +               [Start]                         +                           | 
//       +                  |                            +                           | 
//       +                  | servoUnityRequestNewWindow +                           | 
//       +                  | -------------------------> |                           | 
//       +                  +                            |     CreateServoWindow     |                 
//       +                  +                            | ------------------------> |                 
//       +                  +                            |                           |    
//       +                  +                            |                    Return with window id
//       +                  +                            |                    and texture handle  
//       +                  +                            |                           |    
//       +                  +                            | <------------------------ |                 
//       +                  +                            |                           +    
//       +                  +                      Record texture handle.            + 
//       |                  +                      Retrieve texture                  + 
//       |                  +                      format and size from              + 
//       |                  +                      texture handle.                   + 
//       |                  +                      Invoke windowCreatedCallback      + 
//       |         OnServoWindowCreated                  |                           + 
//       | <-------------------------------------------- |                           + 
//       |   WasCreated     +                            +                           |
//       | ---------------> |                            +                           | 
//       |                  |                            +                           | 
//      ...                ...                       ...                         ...  


using System;
using System.Collections;
using UnityEngine;

public class ServoUnityController : MonoBehaviour
{
    public enum SERVO_UNITY_LOG_LEVEL
    {
        SERVO_UNITY_LOG_LEVEL_DEBUG = 0,
        SERVO_UNITY_LOG_LEVEL_INFO,
        SERVO_UNITY_LOG_LEVEL_WARN,
        SERVO_UNITY_LOG_LEVEL_ERROR,
        SERVO_UNITY_LOG_LEVEL_REL_INFO
    }

    [SerializeField] private SERVO_UNITY_LOG_LEVEL currentLogLevel = SERVO_UNITY_LOG_LEVEL.SERVO_UNITY_LOG_LEVEL_INFO;

    public ServoUnityPlugin Plugin => servo_unity_plugin;

    // Main reference to the plugin functions. Created in OnEnable(), destroyed in OnDisable().
    private ServoUnityPlugin servo_unity_plugin = null;

    // Connections.
    private ServoUnityNavbarController navbarController = null;
    private Camera mainCamera;
    private ServoUnityIME ime = null;

    // Preferences.
    public bool DontCloseNativeWindowOnClose = false;
    public string Homepage = "https://servo.org/";
    [Tooltip("The user agent string passed with every request. If empty, Servo will use a suitable default.")]
    public string UserAgent = "";

    private bool waitingForShutdown = false;

    [NonSerialized] public ServoUnityWindow NavbarWindow = null;

    //
    // Static handlers for callbacks from plugin.
    //

    [AOT.MonoPInvokeCallback(typeof(ServoUnityPluginWindowCreatedCallback))]
    static void OnServoWindowCreated(int uid, int windowIndex, int widthPixels, int heightPixels, int formatNative)
    {
        TextureFormat format = ServoUnityTextureUtils.GetTextureFormatFromNativeTextureFormat(formatNative);
        if (widthPixels == 0 || heightPixels == 0 || format == (TextureFormat)0)
        {
            Debug.LogError("OnServoWindowCreated got invalid format.");
            return;
        }

        // If we initiated the creation of the new window, this find operation will succeed. But
        // if Servo initiated the creation, then there isn't yet a Unity object backing it, so
        // we'll need to create one.
        ServoUnityWindow window = ServoUnityWindow.FindWindowWithUID(uid);
        if (window == null)
        {
            ServoUnityController suc = FindObjectOfType<ServoUnityController>(); // Create it on the same gameobject holding the ServoUnityController.
            if (!suc)
            {
                Debug.LogError("ServoUnityController.OnServoWindowCreated: Couldn't find a ServoUnityController.");
                return;
            }
            window = ServoUnityWindow.CreateNewInParent(suc.transform.parent.gameObject);
        }

        window.WasCreated(windowIndex, widthPixels, heightPixels, format);
    }

    [AOT.MonoPInvokeCallback(typeof(ServoUnityPluginWindowResizedCallback))]
    static void OnServoWindowResized(int uid, int widthPixels, int heightPixels)
    {
        ServoUnityWindow window = ServoUnityWindow.FindWindowWithUID(uid);
        if (window == null)
        {
            Debug.LogError("ServoUnityController.OnFxWindowResized: Received update request for a window that doesn't exist.");
            return;
        }

        window.WasResized(widthPixels, heightPixels);
    }

    [AOT.MonoPInvokeCallback(typeof(ServoUnityPluginBrowserEventCallback))]
    static void OnServoBrowserEvent(int uid, int eventType, int eventData0, int eventData1, System.String eventDataS)
    {
        ServoUnityWindow window = ServoUnityWindow.FindWindowWithUID(uid);
        if (window == null)
        {
            Debug.LogError("ServoUnityController.OnServoBrowserEvent: Received event for a window that doesn't exist.");
            return;
        }

        ServoUnityController suc = FindObjectOfType<ServoUnityController>(); // Create it on the same gameobject holding the ServoUnityController.
        if (!suc)
        {
            Debug.LogError("ServoUnityController.OnServoBrowserEvent: Couldn't find a ServoUnityController.");
            return;
        }

        switch ((ServoUnityPlugin.ServoUnityBrowserEventType)eventType)
        {
            case ServoUnityPlugin.ServoUnityBrowserEventType.NOP:
                break;
            case ServoUnityPlugin.ServoUnityBrowserEventType.Shutdown:
                // Browser has shut down.
                suc.waitingForShutdown = false;
                break;
            case ServoUnityPlugin.ServoUnityBrowserEventType.LoadStateChanged:
                {
                    Debug.Log($"Servo browser event: load {(eventData0 == 1 ? "began" : "ended")}.");
                    if (suc.navbarController) suc.navbarController.OnLoadStateChanged(eventData0 == 1);
                }
                break;
            case ServoUnityPlugin.ServoUnityBrowserEventType.IMEStateChanged:
                {
                    Debug.Log($"Servo browser event: {((eventData0 == 1 || eventData0 == 2) ? "show" : "hide")} IME with text=\"" + eventDataS + "\", textIndex=" + eventData1 + ", multiline=" + (eventData0 == 2));
                    if (suc.ime != null)
                    {
                        if (eventData0 == 1 || eventData0 == 2) suc.ime.ShowIMEForWindowWithInitialText(window.WindowIndex, eventDataS, eventData1, (eventData0 == 2));
                        else suc.ime.HideIME();
                    }
                }
                break;
            case ServoUnityPlugin.ServoUnityBrowserEventType.FullscreenStateChanged:
                {
                    switch (eventData0)
                    {
                        case 0:
                            // Will enter fullscreen. Should e.g. hide windows and other UI.
                            Debug.Log("Servo browser event: will enter fullscreen.");
                            break;
                        case 1:
                            // Did enter fullscreen. Should e.g. show an "exit fullscreen" control.
                            Debug.Log("Servo browser event: did enter fullscreen.");
                            break;
                        case 2:
                            // Will exit fullscreen. Should e.g. hide "exit fullscreen" control.
                            Debug.Log("Servo browser event: will exit fullscreen.");
                            break;
                        case 3:
                            // Did exit fullscreen. Should e.g. show windows and other UI.
                            Debug.Log("Servo browser event: did exit fullscreen.");
                            break;
                        default:
                            break;
                    }
                }
                break;
            case ServoUnityPlugin.ServoUnityBrowserEventType.HistoryChanged:
                {
                    Debug.Log($"Servo browser event: history changed, {(eventData0 == 1 ? "can" : "can't")} go back, {(eventData1 == 1 ? "can" : "can't")} go forward.");
                    suc.navbarController.OnHistoryChanged(eventData0 == 1, eventData1 == 1);
                }
                break;
            case ServoUnityPlugin.ServoUnityBrowserEventType.TitleChanged:
                {
                    Debug.Log("Servo browser event: title changed.");
                    suc.navbarController.OnTitleChanged(suc.servo_unity_plugin.ServoUnityGetWindowTitle(window.WindowIndex));
                }
                break;
            case ServoUnityPlugin.ServoUnityBrowserEventType.URLChanged:
                {
                    Debug.Log("Servo browser event: URL changed.");
                    suc.navbarController.OnURLChanged(suc.servo_unity_plugin.ServoUnityGetWindowURL(window.WindowIndex));
                }
                break;
            default:
                Debug.Log("Servo browser event: unknown event.");
                break;

        }
    }

    //
    // MonoBehavior methods.
    //

    void Awake()
    {
        Debug.Log("ServoUnityController.Awake())");
        navbarController = FindObjectOfType<ServoUnityNavbarController>();
        ime = FindObjectOfType<ServoUnityIME>();
        mainCamera = Camera.main;
    }

    [AOT.MonoPInvokeCallback(typeof(ServoUnityPluginLogCallback))]
    public static void Log(System.String msg)
    {
        if (msg.EndsWith(Environment.NewLine)) msg = msg.Substring(0, msg.Length - Environment.NewLine.Length); // Trim any final newline.
        if (msg.StartsWith("[error]", StringComparison.Ordinal)) Debug.LogError(msg);
        else if (msg.StartsWith("[warning]", StringComparison.Ordinal)) Debug.LogWarning(msg);
        else Debug.Log(msg); // includes [info] and [debug].
    }

    void OnEnable()
    {
        Debug.Log("ServoUnityController.OnEnable()");

        servo_unity_plugin = new ServoUnityPlugin();

        Application.runInBackground = true;

        // Register the log callback.
        switch (Application.platform)
        {
            case RuntimePlatform.OSXEditor: // Unity Editor on OS X.
            case RuntimePlatform.OSXPlayer: // Unity Player on OS X.
            case RuntimePlatform.WindowsEditor: // Unity Editor on Windows.
            case RuntimePlatform.WindowsPlayer: // Unity Player on Windows.
            case RuntimePlatform.LinuxEditor:
            case RuntimePlatform.LinuxPlayer:
            case RuntimePlatform.WSAPlayerX86: // Unity Player on Windows Store X86.
            case RuntimePlatform.WSAPlayerX64: // Unity Player on Windows Store X64.
            case RuntimePlatform.WSAPlayerARM: // Unity Player on Windows Store ARM.
            case RuntimePlatform.Android: // Unity Player on Android.
            case RuntimePlatform.IPhonePlayer: // Unity Player on iOS.
                servo_unity_plugin.ServoUnityRegisterLogCallback(Log);
                servo_unity_plugin.ServoUnitySetLogLevel((int)currentLogLevel);
                break;
            default:
                break;
        }

        // Give the plugin a place to look for resources.

        string resourcesPath = Application.streamingAssetsPath;
        servo_unity_plugin.ServoUnitySetResourcesPath(resourcesPath);

        // Set any launch-time parameters.
        if (DontCloseNativeWindowOnClose)
            servo_unity_plugin.ServoUnitySetParamBool(ServoUnityPlugin.ServoUnityParam.b_CloseNativeWindowOnClose, false);
        if (!String.IsNullOrEmpty(Homepage))
            servo_unity_plugin.ServoUnitySetParamString(ServoUnityPlugin.ServoUnityParam.s_Homepage, Homepage);

        // Set the reference to the plugin in any other objects in the scene that need it.
        ServoUnityWindow[] servoUnityWindows = FindObjectsOfType<ServoUnityWindow>();
        foreach (ServoUnityWindow w in servoUnityWindows)
        {
            w.servo_unity_plugin = servo_unity_plugin;
        }
    }

    void OnDisable()
    {
        Debug.Log("ServoUnityController.OnDisable()");

        // Clear the references to the plugin in any other objects in the scene that have it.
        ServoUnityWindow[] servoUnityWindows = FindObjectsOfType<ServoUnityWindow>();
        foreach (ServoUnityWindow w in servoUnityWindows)
        {
            w.servo_unity_plugin = null;
        }

        servo_unity_plugin.ServoUnitySetResourcesPath(null);

        // Since we might be going away, tell users of our Log function
        // to stop calling it.
        switch (Application.platform)
        {
            case RuntimePlatform.OSXEditor:
            case RuntimePlatform.OSXPlayer:
            case RuntimePlatform.WindowsEditor:
            case RuntimePlatform.WindowsPlayer:
            case RuntimePlatform.LinuxEditor:
            case RuntimePlatform.LinuxPlayer:
            case RuntimePlatform.WSAPlayerX86:
            case RuntimePlatform.WSAPlayerX64:
            case RuntimePlatform.WSAPlayerARM:
            case RuntimePlatform.Android:
            case RuntimePlatform.IPhonePlayer:
                servo_unity_plugin.ServoUnityRegisterLogCallback(null);
                break;
            default:
                break;
        }

        servo_unity_plugin = null;
    }

    void Start()
    {
        Debug.Log("ServoUnityController.Start()");
        Debug.Log("Plugin version " + servo_unity_plugin.ServoUnityGetVersion());

        servo_unity_plugin.ServoUnityInit(OnServoWindowCreated, OnServoWindowResized, OnServoBrowserEvent, UserAgent);
    }

    void Update()
    {
        servo_unity_plugin.ServoUnityFlushLog();
    }

    private void OnApplicationQuit()
    {
        Debug.Log("ServoUnityController.OnApplicationQuit()");

        ServoUnityWindow[] servoUnityWindows = FindObjectsOfType<ServoUnityWindow>();
        foreach (ServoUnityWindow w in servoUnityWindows)
        {
            w.CleanupRenderer();
        }

        // Because Servo cleanup must happen on the GPU thread, we must wait until
        // the GPU thread has time to process the cleanup. We assume that the GPU thread
        // continues to be called while we block the UI thread in a spinlock servicing
        // window events in the plugn. We'll exit the spinlock when one of those events
        // is a callback to signal the browser shutdown, or when a timeout is reached.
        // If we have more than one window, we'll need to change this logic to
        // wait for all windows to be shut down. At the moment, it will continue
        // as soon as the first is done.

        System.Diagnostics.Stopwatch stopWatch = new System.Diagnostics.Stopwatch();
        stopWatch.Start();
        if (servoUnityWindows.Length > 0)
        {
            waitingForShutdown = true;
            do
            {
                servo_unity_plugin.ServoUnityServiceWindowEvents(servoUnityWindows[0].WindowIndex);
            } while (waitingForShutdown == true && stopWatch.ElapsedMilliseconds < 2000);
            stopWatch.Stop();
            if (waitingForShutdown)
            {
                Debug.LogWarning("Timed out waiting for browser shutdown.");
            }
        }

        // Allow any events from the browser shutdown on the GPU thread to make it into the log.
        servo_unity_plugin.ServoUnityFlushLog();

        // Now safe to close the windows.
        foreach (ServoUnityWindow w in servoUnityWindows)
        {
            w.Close();
        }

        servo_unity_plugin.ServoUnityFinalise();
    }

    public SERVO_UNITY_LOG_LEVEL LogLevel
    {
        get { return currentLogLevel; }

        set
        {
            currentLogLevel = value;
            servo_unity_plugin.ServoUnitySetLogLevel((int)currentLogLevel);
        }
    }

}