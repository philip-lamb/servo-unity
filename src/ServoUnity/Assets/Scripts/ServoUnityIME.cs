using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ServoUnityIME : MonoBehaviour
{
    ServoUnityController suc = null;

    private UnityEngine.UI.InputField[] inputFields = null;

    public enum ServoUnityIMEType
    {
        UnityGUIDefault,
        UnitySystemVirtual,
    }

    [SerializeField]
    private ServoUnityIMEType _type = ServoUnityIMEType.UnityGUIDefault;
    
    private void setIMEType(ServoUnityIMEType type)
    {
        _type = type;
    }

    public ServoUnityIMEType IMEType
    {
        get => _type;
        set
        {
            if (_type != value)
            {
                setIMEType(value);
            }
        }
    }

    private bool _active = false;
    private int _windowIndex = 0;

    private void sendKeyPress(ServoUnityPlugin.ServoUnityKeyCode keyCode, int character)
    {
        if (keyCode == ServoUnityPlugin.ServoUnityKeyCode.Backspace) Debug.Log("Sending keypress backspace");
        else if (keyCode == ServoUnityPlugin.ServoUnityKeyCode.Delete) Debug.Log("Sending keypress delete");
        else if (keyCode == ServoUnityPlugin.ServoUnityKeyCode.LeftArrow) Debug.Log("Sending keypress left arrow");
        else if (keyCode == ServoUnityPlugin.ServoUnityKeyCode.RightArrow) Debug.Log("Sending keypress right arrow");
        else if (keyCode == ServoUnityPlugin.ServoUnityKeyCode.Return) Debug.Log("Sending keypress return");
        else if (keyCode == ServoUnityPlugin.ServoUnityKeyCode.Escape) Debug.Log("Sending keypress escape");
        else if (keyCode == ServoUnityPlugin.ServoUnityKeyCode.Character) Debug.Log("Sending keypress " + character);
        else Debug.Log("Sending keypress something else");

        suc.Plugin.ServoUnityKeyEvent(_windowIndex, true, keyCode, character);
        suc.Plugin.ServoUnityKeyEvent(_windowIndex, false, keyCode, character);
    }

    public bool IMEActive {
        get
        {
            bool inputFieldActive = false;
            foreach (UnityEngine.UI.InputField inputField in inputFields)
            {
                if (inputField.isFocused)
                {
                    inputFieldActive = true;
                    break;
                }
            }
            return _active || inputFieldActive;
        }
    }

    public void ShowIMEForWindowWithInitialText(int index, string initialIMEText, int initialIMETextCaretIndex, bool multiline)
    {
        if (!_active)
        {
            _active = true;
            _windowIndex = index;
        }
    }

    public void HideIME()
    {
        if (_active) {
            OnIMEDismissed();
        }
    }

    /// <summary>
    /// Should be called by the IME when the user has cancelled the IME by user action
    /// e.g. cancelling input by selecting a cancel control, or completing input, or
    /// automatically by selecting some other UI element.
    /// </summary>
    public void OnIMEDismissed()
    {
        suc.Plugin.ServoUnityWindowBrowserControlEvent(_windowIndex, ServoUnityPlugin.ServoUnityWindowBrowserControlEventID.IMEDismissed, 0, 0, null);
        _windowIndex = 0;
        _active = false;
    }

    /// <summary>
    /// Should be called by the IME when the user has completed input.
    /// </summary>
    public void OnIMECompleted()
    {
        sendKeyPress(ServoUnityPlugin.ServoUnityKeyCode.Return, 0);
        OnIMEDismissed();
    }

    private void Awake()
    {
        suc = FindObjectOfType<ServoUnityController>();
        // Build list of all TMPro.TMP_InputField in scene.
        inputFields = FindObjectsOfType<UnityEngine.UI.InputField>();

        setIMEType(_type);
    }

    void OnDestroy()
    {
        inputFields = null;
        suc = null;
    }
    void OnGUI()
    {
        if (_type == ServoUnityIMEType.UnityGUIDefault && _active)
        {
            Event e = Event.current;
            if (e.isKey)
            {
                Debug.Log("ServoUnityController.OnGUI() got Event.isKey");
                ServoUnityPlugin.ServoUnityKeyCode keyCode;
                int character = 0;
                switch (e.keyCode)
                {
                    case KeyCode.Backspace: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Backspace; break;
                    case KeyCode.Delete: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Delete; break;
                    case KeyCode.Tab: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Tab; break;
                    case KeyCode.Clear: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Clear; break;
                    case KeyCode.Return: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Return; break;
                    case KeyCode.Pause: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Pause; break;
                    case KeyCode.Escape: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Escape; break;
                    case KeyCode.Space: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Space; break;
                    case KeyCode.UpArrow: keyCode = ServoUnityPlugin.ServoUnityKeyCode.UpArrow; break;
                    case KeyCode.DownArrow: keyCode = ServoUnityPlugin.ServoUnityKeyCode.DownArrow; break;
                    case KeyCode.RightArrow: keyCode = ServoUnityPlugin.ServoUnityKeyCode.RightArrow; break;
                    case KeyCode.LeftArrow: keyCode = ServoUnityPlugin.ServoUnityKeyCode.LeftArrow; break;
                    case KeyCode.Insert: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Insert; break;
                    case KeyCode.Home: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Home; break;
                    case KeyCode.End: keyCode = ServoUnityPlugin.ServoUnityKeyCode.End; break;
                    case KeyCode.PageUp: keyCode = ServoUnityPlugin.ServoUnityKeyCode.PageUp; break;
                    case KeyCode.PageDown: keyCode = ServoUnityPlugin.ServoUnityKeyCode.PageDown; break;
                    case KeyCode.F1: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F1; break;
                    case KeyCode.F2: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F2; break;
                    case KeyCode.F3: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F3; break;
                    case KeyCode.F4: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F4; break;
                    case KeyCode.F5: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F5; break;
                    case KeyCode.F6: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F6; break;
                    case KeyCode.F7: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F7; break;
                    case KeyCode.F8: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F8; break;
                    case KeyCode.F9: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F9; break;
                    case KeyCode.F10: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F10; break;
                    case KeyCode.F11: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F11; break;
                    case KeyCode.F12: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F12; break;
                    case KeyCode.F13: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F13; break;
                    case KeyCode.F14: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F14; break;
                    case KeyCode.F15: keyCode = ServoUnityPlugin.ServoUnityKeyCode.F15; break;
                    case KeyCode.Numlock: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Numlock; break;
                    case KeyCode.CapsLock: keyCode = ServoUnityPlugin.ServoUnityKeyCode.CapsLock; break;
                    case KeyCode.ScrollLock: keyCode = ServoUnityPlugin.ServoUnityKeyCode.ScrollLock; break;
                    case KeyCode.RightShift: keyCode = ServoUnityPlugin.ServoUnityKeyCode.RightShift; break;
                    case KeyCode.LeftShift: keyCode = ServoUnityPlugin.ServoUnityKeyCode.LeftShift; break;
                    case KeyCode.RightControl: keyCode = ServoUnityPlugin.ServoUnityKeyCode.RightControl; break;
                    case KeyCode.LeftControl: keyCode = ServoUnityPlugin.ServoUnityKeyCode.LeftControl; break;
                    case KeyCode.RightAlt: keyCode = ServoUnityPlugin.ServoUnityKeyCode.RightAlt; break;
                    case KeyCode.LeftAlt: keyCode = ServoUnityPlugin.ServoUnityKeyCode.LeftAlt; break;
                    case KeyCode.LeftCommand: keyCode = ServoUnityPlugin.ServoUnityKeyCode.LeftCommand; break;
                    case KeyCode.LeftWindows: keyCode = ServoUnityPlugin.ServoUnityKeyCode.LeftWindows; break;
                    case KeyCode.RightCommand: keyCode = ServoUnityPlugin.ServoUnityKeyCode.RightCommand; break;
                    case KeyCode.RightWindows: keyCode = ServoUnityPlugin.ServoUnityKeyCode.RightWindows; break;
                    case KeyCode.AltGr: keyCode = ServoUnityPlugin.ServoUnityKeyCode.AltGr; break;
                    case KeyCode.Help: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Help; break;
                    case KeyCode.Print: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Print; break;
                    case KeyCode.SysReq: keyCode = ServoUnityPlugin.ServoUnityKeyCode.SysReq; break;
                    case KeyCode.Break: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Break; break;
                    case KeyCode.Menu: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Menu; break;
                    case KeyCode.Keypad0: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad0; break;
                    case KeyCode.Keypad1: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad1; break;
                    case KeyCode.Keypad2: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad2; break;
                    case KeyCode.Keypad3: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad3; break;
                    case KeyCode.Keypad4: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad4; break;
                    case KeyCode.Keypad5: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad5; break;
                    case KeyCode.Keypad6: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad6; break;
                    case KeyCode.Keypad7: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad7; break;
                    case KeyCode.Keypad8: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad8; break;
                    case KeyCode.Keypad9: keyCode = ServoUnityPlugin.ServoUnityKeyCode.Keypad9; break;
                    case KeyCode.KeypadPeriod: keyCode = ServoUnityPlugin.ServoUnityKeyCode.KeypadPeriod; break;
                    case KeyCode.KeypadDivide: keyCode = ServoUnityPlugin.ServoUnityKeyCode.KeypadDivide; break;
                    case KeyCode.KeypadMultiply: keyCode = ServoUnityPlugin.ServoUnityKeyCode.KeypadMultiply; break;
                    case KeyCode.KeypadMinus: keyCode = ServoUnityPlugin.ServoUnityKeyCode.KeypadMinus; break;
                    case KeyCode.KeypadPlus: keyCode = ServoUnityPlugin.ServoUnityKeyCode.KeypadPlus; break;
                    case KeyCode.KeypadEnter: keyCode = ServoUnityPlugin.ServoUnityKeyCode.KeypadEnter; break;
                    case KeyCode.KeypadEquals: keyCode = ServoUnityPlugin.ServoUnityKeyCode.KeypadEquals; break;
                    default:
                        if (e.character != 0)
                        {
                            keyCode = ServoUnityPlugin.ServoUnityKeyCode.Character;
                            character = e.character;
                        }
                        else
                        {
                            return;
                        }
                        break;
                }
                if (e.type == EventType.KeyDown)
                {
                    suc.Plugin.ServoUnityKeyEvent(_windowIndex, true, keyCode, character);
                }
                else if (e.type == EventType.KeyUp)
                {
                    suc.Plugin.ServoUnityKeyEvent(_windowIndex, false, keyCode, character);
                }
                e.Use();
            } // e.isKey
        } // _type == ServoUnityKeyboardType.UnityGUIDefault && _active
    }

}
