#include "include/native_context_menu_ng/native_context_menu_ng_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "native_context_menu_ng_plugin.h"

void NativeContextMenuNgPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  native_context_menu_ng::NativeContextMenuNgPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
