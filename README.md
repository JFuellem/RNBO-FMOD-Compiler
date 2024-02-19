# RNBO-FMOD-Compiler

This App is built to make plugins for the FMOD Engine with Cycling 74 RNBO as easy as possible.

# Needed Steps RNBO
1. Name your Plugin by adding a |param| object with argument ```NAME_<PluginName>```
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
  
5. Export it as a C++ Source Code by choosing an output directory. No other modifications needed. Make sure, the export name stays rnbo_source.cpp.
  
