import 'dart:typed_data';

import 'package:flutter/src/services/message_codec.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:native_context_menu_ng/native_context_menu_ng.dart';
import 'package:native_context_menu_ng/native_context_menu_ng_platform_interface.dart';
import 'package:native_context_menu_ng/native_context_menu_ng_method_channel.dart';
import 'package:native_context_menu_ng/native_menu.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockNativeContextMenuNgPlatform with MockPlatformInterfaceMixin implements NativeContextMenuNgPlatform {
  @override
  Future<int?> showNativeContextMenu(NativeMenu menu) {
    return Future.value(-1);
  }

  @override
  void setMethodCallHandler(Future Function(MethodCall call)? handler) {
    // TODO: implement setMethodCallHandler
  }
}

void main() {
  final NativeContextMenuNgPlatform initialPlatform = NativeContextMenuNgPlatform.instance;

  test('$MethodChannelNativeContextMenuNg is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelNativeContextMenuNg>());
  });

  test('showNativeContextMenu', () async {
    NativeContextMenuNg nativeContextMenuNgPlugin = NativeContextMenuNg();
    MockNativeContextMenuNgPlatform fakePlatform = MockNativeContextMenuNgPlatform();
    NativeContextMenuNgPlatform.instance = fakePlatform;

    NativeMenu nativeMenu = NativeMenu();
    NativeMenuItem nativeMenuItem1 = NativeMenuItem.simple(title: "title", action: "action");
    NativeMenuItem nativeSeparator = NativeMenuItem.separator();
    NativeMenuItem nativeWithIcon = NativeMenuItem.withIcon(title: "title", action: "action", icon: "iconPath");
    Uint8List unit8list = Uint8List(10);
    NativeMenuItem nativeWithRawIcon = NativeMenuItem.withRawIcon(title: "title", action: "action", rawIcon: unit8list);
    nativeMenu.addItem(nativeMenuItem1);
    nativeMenu.addItem(nativeSeparator);
    nativeMenu.addItem(nativeWithIcon);
    nativeMenu.addItem(nativeWithRawIcon);

    expect(await nativeContextMenuNgPlugin.showNativeContextMenu(nativeMenu), 0);
  });
}
