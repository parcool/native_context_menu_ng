#ifndef FLUTTER_PLUGIN_NATIVE_CONTEXT_MENU_NG_PLUGIN_H_
#define FLUTTER_PLUGIN_NATIVE_CONTEXT_MENU_NG_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/encodable_value.h>

#include <memory>
#include <vector>
#include <windef.h>

using namespace std;

namespace native_context_menu_ng {

class NativeContextMenuNgPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  NativeContextMenuNgPlugin();

  virtual ~NativeContextMenuNgPlugin();

  // Disallow copy and assign.
  NativeContextMenuNgPlugin(const NativeContextMenuNgPlugin&) = delete;
  NativeContextMenuNgPlugin& operator=(const NativeContextMenuNgPlugin&) = delete;

 private:
  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  
  std::wstring ConvertStringToWideString(const std::string& str);
  LPCWSTR stringToLPCWSTR(const std::string& str);
  std::string LPCWSTRToString(LPCWSTR wideStr);

  int getBestIconSize();
  HRESULT CreateHBitmapFromPNGBytes(const std::vector<uint8_t>& pngBytes, HBITMAP& hBitmap, int width, int height);
  HBITMAP LoadPngImage(const std::string& filePath, int width, int height);
  void buildPopupMenu(HMENU hMenu, std::vector<flutter::EncodableValue> items);

  std::optional<LRESULT> HandleWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
};

}  // namespace native_context_menu_ng

#endif  // FLUTTER_PLUGIN_NATIVE_CONTEXT_MENU_NG_PLUGIN_H_
