## About this project

This project constitutes a Unity native plugin and a set of Unity C# script components allow third parties to incorporate Servo browser windows into Unity scenes.

For more background information, see the blog post at https://blog.mozvr.com/a-browser-plugin-for-unity/

## Structure of this repository

1. The Unity project is `src/ServoUnity`, with a sample scene at `src/ServoUnity/Assets/ExampleScene.unity`.
2. Native code for the Unity plugin is in `src/ServoUnityPlugin`.
3. The compiled plugin will be placed in `src/ServoUnity/Assets/Plugins`.
4. The Unity C# scripts designed to be used by the user's application are in `src/ServoUnity/Assets/Scripts`.

## License

[The license file](License) sets out the full license text.

The code is licensed under the MPL2.

This license is compatible with use in a proprietary and/or commercial application.

## Building from source

### Prerequisites

What | Minimum version | Where to download 
---- | --------------- | ------------
Unity | 2019.3 | <https://unity.com>
For macOS: Xcode tools |  | <https://developer.apple.com>
For Windows: Visual Studio 2019 (Community or Professional) |  | <https://visualstudio.microsoft.com/vs/>

During this development phase of the project, only macOS is supported. 

### libsimpleservo2

Servo itself enters the plugin through the wrapper library `libsimpleservo2`. You can find a binary build of libsimpleservo2 at https://github.com/philip-lamb/servo/releases, or you can build it from source. This fork of the servo repository will soon be merged back to the main servo repo and these instructions will be amended once that is complete.

Build libsimpleservo2 from source:
1. `git clone --branch phil-ss2-headless https://github.com/philip-lamb/servo.git`
2. `cd servo`
3. `./mach bootstrap`
4. `./mach build --libsimpleservo2`
The release libraries will be built by default to path `target/release`.

### The servo-unity plugin build

The macOS Xcode project for the plugin is at `src/ServoUnityPlugin/macOS/servo_unity.xcodeproj`, or the Windows Visual Studio 2019 project is at `src/ServoUnityPlugin/Windows/servo_unity.sln`. Compiling the project requires linking to Unity's plugin headers which are normally contained inside the Unity application bundle (macOS) or inside the Unity program directory (Windows). Check that the build setting for header search paths is correct for the version of Unity installed on your system.

On macOS, prior to building, a build step removes any previous plugin build (`servounity.bundle`) from the Unity project's `Plugins` folder. The Xcode project builds the plugin bundle directly into the same folder. If you wish to change this behaviour, uncheck "deployment postprocessing" in the Xcode build settings.

On macOS, libsimpleservo2 and the required GStreamer plugins will be copied into the servo_unity bundle. On Windows, they will be copied into the same directory as the plugin DLL.

## Operating the plugin inside the Unity Editor

The plugin can run inside the Unity Editor, but the plugin can be run and stopped once per Editor session. (This is due to the fact that Unity does not unload and reload native plugins between runs in the Editor.) You'll need to quit and relaunch the Editor before running again.

## Debugging

To launch with Rust debug logging, use this macOS script (Unity will inherit the environment from Unity Hub):

```
#!/bin/bash
export RUST_BACKTRACE=1
export RUST_LOG=debug
nohup "/Applications/Unity Hub.app/Contents/MacOS/Unity Hub" &>/dev/null &
```

The Unity log, including plugin log output, Unity log, and stdout/stderr will be written to either `~/Library/Logs/Unity/Editor.log` or `~/Library/Logs/Mozilla/ServoUnity/Player.log`.
