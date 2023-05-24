import 'package:flutter/services.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'native_context_menu_ng_method_channel.dart';
import 'native_menu.dart';

abstract class NativeContextMenuNgPlatform extends PlatformInterface {
  /// Constructs a NativeContextMenuNgPlatform.
  NativeContextMenuNgPlatform() : super(token: _token);

  static final Object _token = Object();

  static NativeContextMenuNgPlatform _instance = MethodChannelNativeContextMenuNg();

  /// The default instance of [NativeContextMenuNgPlatform] to use.
  ///
  /// Defaults to [MethodChannelNativeContextMenuNg].
  static NativeContextMenuNgPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [NativeContextMenuNgPlatform] when
  /// they register themselves.
  static set instance(NativeContextMenuNgPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<int?> showNativeContextMenu(NativeMenu menu) {
    throw UnimplementedError('showNativeContextMenu() has not been implemented.');
  }

  void setMethodCallHandler(Future<dynamic> Function(MethodCall call)? handler) {
    throw UnimplementedError('setMethodCallHandler() has not been implemented.');
  }
}
