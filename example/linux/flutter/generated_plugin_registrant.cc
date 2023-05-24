//
//  Generated file. Do not edit.
//

// clang-format off

#include "generated_plugin_registrant.h"

#include <native_context_menu_ng/native_context_menu_ng_plugin.h>

void fl_register_plugins(FlPluginRegistry* registry) {
  g_autoptr(FlPluginRegistrar) native_context_menu_ng_registrar =
      fl_plugin_registry_get_registrar_for_plugin(registry, "NativeContextMenuNgPlugin");
  native_context_menu_ng_plugin_register_with_registrar(native_context_menu_ng_registrar);
}
