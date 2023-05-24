import 'package:flutter/foundation.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/widgets.dart';

import 'native_context_menu_ng.dart';
import 'native_menu.dart';

class NativeContextMenuWidget extends StatefulWidget {
  final NativeMenu menu;
  final Widget child;
  final Function(dynamic) actionCallback;
  final Function(dynamic)? otherCallback;

  const NativeContextMenuWidget({super.key, required this.child, required this.menu, required this.actionCallback, this.otherCallback});

  @override
  State<NativeContextMenuWidget> createState() => _NativeContextMenuWidgetState();
}

class _NativeContextMenuWidgetState extends State<NativeContextMenuWidget> {
  final _nativeContextMenuNgPlugin = NativeContextMenuNg();
  bool isMouseRightClickDown = false;

  void showNativeContextMenu() async {
    _nativeContextMenuNgPlugin.setMethodCallHandler((call) async {
      switch (call.method) {
        case "onItemClicked":
          final arg = call.arguments as Map<dynamic, dynamic>;
          final action = arg["action"];
          widget.actionCallback(action);
          break;
        case "menuDidClose":
          widget.otherCallback?.call(call.method);
          break;
        default:
          break;
      }
    });
    try {
      await _nativeContextMenuNgPlugin.showNativeContextMenu(widget.menu);
    } catch (e) {
      if (kDebugMode) {
        print(e);
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        Positioned.fill(
          child: Listener(
            onPointerDown: (e) {
              isMouseRightClickDown = e.kind == PointerDeviceKind.mouse && e.buttons == kSecondaryMouseButton;
            },
            onPointerUp: (e) async {
              if (!isMouseRightClickDown) {
                return;
              }
              showNativeContextMenu();
            },
            child: widget.child,
          ),
        ),
      ],
    );
  }
}
