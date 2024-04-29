# RNBO-FMOD-Compiler

This App is built to make plugins for the FMOD Engine with Cycling 74 RNBO as easy as possible.

## Needed Steps RNBO

1. Name your Plugin by adding a |param| object with argument ```Name_<PluginName>```
2. Create an RNBO patch as you like either as an instrument or effect. The presence of in~ and out~ objects determines the type.
3. Add |param| objects that you need. min, max, value and unit attributes will automatically be transferred.
4. Optionally add any or multiple of the following |inports| that will receive 3d-Attributes from the source as a list:

  | Argument | Explanation             |
  |----------|-------------------------|
  | rel_pos  | Relative Position       |
  | rel_vel  | Relative Velocity       |
  | rel_forw | Relative Forward Vector |
  | rel_up   | Relative Up Vector      |
  | abs_pos  | Absolute Position       |
  | abs_vel  | Absolute Velocity       |
  | abs_forw | Absolute Forward Vector |
  | abs_up   | Absolute Up Vector      |

  If the plugin is added to an event as an effect, it automatically turns into a 3D event (even without a spatialiser). For instrument plugins, you must add a spatialiser to preview the effect in FMDO Studio.
  
5. Export it as a C++ Source Code by choosing an output directory. No other modifications are needed. Make sure, the export name stays rnbo_source.cpp.
  
## Needed Steps Compiler

1. Download the Compiler and open it.
2. On Mac, it should prompt you to download the Developer tools, if you haven't installed them already. On Windows, the Compiler is included.
3. Download the FMOD Engine API. You can use the link inside the App. Only the inc-folder is needed.
4. Specify where you exported the RNBO-Project. It should automatically recognize the Plugin-Name.
5. Specify an Export Directory. If none is specified. The plugin will be put, where the Compiler resides.
6. Hit Export and wait. The first time can take quite long.
7. Move the plugin to a plugin folder of an FMOD Project or to the plugin folder of the application itself.


## Things to consider
The performance of these plugins is worse than the ones FMOD provides. You can test the performance with the profiler.

If you find any bugs or something isn't working as expected, feel free to add a bug report or contact me.

If you're feeling generous, you can donate [here](https://www.paypal.com/donate/?business=5WX6KRT4HFEU2&no_recurring=1&currency_code=CHF).
