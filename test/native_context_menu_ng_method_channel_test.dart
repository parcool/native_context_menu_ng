import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:native_context_menu_ng/native_context_menu_ng_method_channel.dart';
import 'package:native_context_menu_ng/native_menu.dart';

void main() {
  MethodChannelNativeContextMenuNg platform = MethodChannelNativeContextMenuNg();
  const MethodChannel channel = MethodChannel('native_context_menu_ng');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(channel, (MethodCall methodCall) async {
      return 0;
    });
  });

  tearDown(() {
    TestDefaultBinaryMessengerBinding.instance.defaultBinaryMessenger.setMockMethodCallHandler(channel, null);
  });

  test('showNativeContextMenu', () async {
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
    expect(await platform.showNativeContextMenu(nativeMenu), 0);
  });
}
