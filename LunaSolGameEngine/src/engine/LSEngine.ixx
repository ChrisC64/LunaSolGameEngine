export module LSEngine;

// Engine Modules //
export import Engine.App;
export import Engine.EngineCodes;
export import Engine.LSCamera;
export import Engine.LSDevice;
export import Engine.LSWindow;
export import Engine.Logger;
export import Engine.Defines;
export import Engine.Shader;
export import Engine.Input;
export import Engine.Renderer;

// Objects not pertaining to engine but could, generic tools that are for all platforms
export import Clock;
export import LSDataLib;
export import Helper.IO;
export import Helper.LSCommonTypes;
export import MathLib;
export import GeometryGenerator;
export import Util;

#ifdef LS_WIN32_BUILD
export import Platform.Windows;
#endif //WINDOWS