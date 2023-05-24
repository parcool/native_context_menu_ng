import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'native_context_menu_ng_platform_interface.dart';
import 'native_menu.dart';

/// An implementation of [NativeContextMenuNgPlatform] that uses method channels.
class MethodChannelNativeContextMenuNg extends NativeContextMenuNgPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('native_context_menu_ng');

  @override
  Future<int?> showNativeContextMenu(NativeMenu menu) {
    return methodChannel.invokeMethod("showNativeContextMenu", menu.toJson());
  }

  @override
  void setMethodCallHandler(Future<dynamic> Function(MethodCall call)? handler) {
    methodChannel.setMethodCallHandler(handler);
  }
}
