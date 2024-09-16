# uevr-index-controls
Valve Index Controls profile and plugin for UEVR

This plugin is a combination of a .json interaction profile and dll plugin for UEVR to fix the broken
valve index controls in openxr. By default, index controller grips are stuck on as long as you're holding
the controllers, the trackpads don't work, the system buttons don't work, and there's no way to bring up
the start/select (menu/back) buttons at all.

This project fixes the controls in the following ways:
- Fixes grips. You now must squeeze them to activate them. (function added by json file)
- Adds haptic to the grips when activated so you feel like you pressed a button (function added by dll file)
- Turns the left track pad into a dpad on touch up,down,left,right (function added by json file)
- Turns the left track pad into a select / back button on hard press (function added by dll file)
- Turns the right track pad into a start / menu button on hard press (function added by dll file)
- Adds haptic to presses on trackpads for start or select (function added by dll file)
- Adds xbox B and Y to touch of right trackpad top of trackpad is Y, bottom is B.
  This is useful for cases wher B is a dodge or such and is hard to do when left thumb is occupied on left stick.
  (Function added json file).

With this plugin, you have a full xbox like controlle without needing any shift buttons with dpad, start, select,
and all buttons available. Since start and select are separated, games that use functionality like holding menu to
bring up map (outer worlds) can work as other methods in UEVR combines these buttons to one.

This is a must for anyone that has a Varjo Aero and index controllers
as this is not a native steamvr device and performs much better on openxr. There may be other uses
for this.

This can be a global install or per game. To make it global:

As of UEVR v1.04, you can create a global plugins folder and copy the json and plugin into them. Create a folder in %appdata%\UnrealVRMod\UEVR\Plugins and another in %appdata%\UnrealVRMod\UEVR\Profiles. Copy the dll into plugins and the json into profiles.

For per game, 
The plugins must be copied into each game directory in %appdata\UnrealVRMod\GAMEPROFILE\Plugins or %appdata%\UnrealVRMod\GAMEPROFILE\Profile


The _interaction_profiles_valve_index_controller.json goes in the profile directory and fixes the controller interactions. 

The IndexHapic.dll goes in the profile plugins directory.

