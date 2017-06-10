# cycle_engine
This is a game engine I've been working for a few months now. Even though it uses C++ middleware, its core is mostly written in C, and is data oriented.

So far, its renderer features:
  - deferred rendering
  - shadow mapping
  - normal mapping
  - parallax occlusion mapping
  - bloom (broken)
  - volumetric lights (broken)
  - translucent (and refractive) surfaces (unfinished)
  - PBR materials

An option to allow it to switch to forward rendering is in consideration.  

Physics engine used:
  - Bullet physics
  
Sound playback (deactivated):
  - OpenAL
    
No scripting so far.


Dependencies:
  - SDL2
  - GLEW
  - Bullet physics
  - Assimp
  - Freetype
  - OpenAL

The repository will be a mess for now for it mirrors exactly my local one. There are some HUGE texture files on the texture folder. Will shrink their size eventually. All the dependencies are in the libs folder, including some the engine actually don't use (FFMPEG and ozz). Will get rid of them eventually. The dev-cpp project is also here, so not much setup is required.

The renderer architecture is undergoing severe modifications, which broke volumetric lights and bloom code (somehow). Those bugs still are to be narrowed down.  
