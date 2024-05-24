export module Platform.Windows;

#ifdef LS_WINDOWS_BUILD
export import D3D11Lib;
export import D3D12Lib;
export import DirectXCommon;
export import DXGIHelper;
export import Win32.ComUtils;
export import Win32.Utils;
export import Platform.Win32Window;
#endif