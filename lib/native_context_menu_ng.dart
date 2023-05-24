import 'package:flutter/services.dart';

import 'native_context_menu_ng_platform_interface.dart';
import 'native_menu.dart';


class NativeContextMenuNg {
  Future<int?> showNativeContextMenu(NativeMenu menu) {
    return NativeContextMenuNgPlatform.instance.showNativeContextMenu(menu);
  }

  void setMethodCallHandler(Future<dynamic> Function(MethodCall call)? handler) {
    NativeContextMenuNgPlatform.instance.setMethodCallHandler(handler);
  }

}
