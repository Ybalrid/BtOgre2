# BtOgre21 - Simple Bullet-Ogre 2.1 connection


BtOgre21 is a simple Bullet-Ogre connection made for Ogre 2.1+

## Attribution of original work

This is a forked project from BtOgre, originally created by nikki93 (as of his github name), and take root form a fork by nyxkn.

The goal is to reorganise the source code of this project, and also do some cleaning.

## Changes made and current state

 - Motion states moves "derivated coordinates" instead of the parent ones
 - Class implementation are not in headers
 - Debug drawer re-implemented for Ogre V2 by using a manual object instead of an array of dynamic renderables
   - The debug drawer also supports every mode of debug drawing Bullet can offer, including color
   - The debug drawer uses an HLMS Unlit datablock created at run time the first time you call it
   - The color of the line is multiplied by a factor the user can set to accomodate HDR rendering pipeline and the way the user wants to deal with color spaces and gamma correction

## Changes planned

  - Rewote the static *mesh to shape converter* to use v2 meshes, or even Item, instead of v1 meshes or Entity object
  - Revisit the animated *mesh to shape converter* but right now Ogre V2 animations and v1 animations co-exist in a weird state
  - Try to implement something for soft body physics.
  - Try to make sure nothing's break if you change the origin of the scene. Ogre support that now by moving the root node. This could cause thing to go quite wrong in the Physics <-> Graphics communication
  - Integrate a motion state that don't actually move the node until told to do so by the user, for multithreaded uses cases. (See Ogre 2.1 samples)

To do theses things, I'll probably need to put my head quite deep into the VAO (Vertex Array Object) part of Ogre, and how it actually uses vertex buffers and everything. Currently, I don't really have a clue. But everything can be learn if you bang your head hard enough on the source code and the documentation... right? :p

--- 

## Current state of this project

Currently, I can't recomment to anybody to actually use this thing. But it's working. You don't have a way to cleanly build it yet, but just compiling these files into your project should do it. 

In the present state, it is relying on Ogre v1 mesh object to generate Bullet body shapes. I want to move that to be fully Ogre v2.

the CMake build on this repo, and the premake / lua things are probably broken as of now, and this project is pretty much a work in progress.

One of the things I will do is make CMake output a project that build BtOgre as either a static or dynamic library properly. I'm currently using BtOgre with the "paste the sources in your project" and that's not that great... ^^"
