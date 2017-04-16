BtOgre21 Demo program

This is a small program that shows how BtOgre works.

It's a bit hackish, it's just the main.cpp file of the old demo modified to actually do something with Ogre 2.1.

It used to run with the example application framework and a frame listener. For now, you can just see one rigidbody falling into another one.

To build this folder, use CMake.

There's no proper installation for BtOgre yet, but following theses steps should make you succeed to run this program


    1 - Build BtOgre somewhere whith CMake (see root directory)
    2 - Generate your build files in another directory (I suggest build, it's here for your convinience)
    3 - While calling CMake, set the following variables : 
        - OGRE_HOME : Your Ogre SDK
        - BULLET_ROOT : Your Bullet3 SDK
        - BTOGRE_INCLIDE : The directory that contains your BtOgre.hpp and other files
        - BTOGRE_LIB : The directory that contains the release build of BtOgre (if your are on windows, look for a "BtOgre21.lib" file, if you are on linux, a "libBtOgre21.a" one)
    4 - Build it
    5 - Copy a valid plugins.cfg files pointing to the plugins you use with Ogre
    6 - Create next to your executable a subfolder called "HLMS" and copy the "Hlms" folder (folder itself -- not the content) inside it
        
