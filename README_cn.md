# native_context_menu_ng
[[English]](README.md)  |  [[中文]](README_cn.md)

native_context_menu_ng是一个flutter插件，给flutter应用提供原生的右键菜单，支持3个主流桌面平台（macOS、Windows、Linux）。

# 特色
* 支持子菜单
* 支持图标（为了平台统一性全部rawIcon暂时只支持png格式）
* 图标大小不用手动指定，它会自动根据平台硬件与缩放设置为最佳大小；
* 支持分割线；
* 支持菜单项禁用/启用；

<img src="screenshot.gif" alt="macOS" />

# 使用示例

1. 先构建菜单
```dart
Future<NativeMenu> initMenu() async {
    //如果有子菜单的话是可以不用设置当前NativeMenuItem的action参数的
    NativeMenuItem itemNew = NativeMenuItem.simple(title: "New");
    //显示asset里的图片
    Uint8List iconText = await VenyoreImageUtil.assetImageToUint8List("assets/images/txt.png");
    Uint8List iconWord = await VenyoreImageUtil.assetImageToUint8List("assets/images/word.png");
    Uint8List iconExcel = await VenyoreImageUtil.assetImageToUint8List("assets/images/excel.png");
    Uint8List iconPdf = await VenyoreImageUtil.assetImageToUint8List("assets/images/pdf.png");
    NativeMenu subMenu = NativeMenu();
    subMenu.addItem(NativeMenuItem.withRawIcon(title: "Text", action: "action_text", rawIcon: iconText));
    subMenu.addItem(NativeMenuItem.withRawIcon(title: "Word", action: "action_word", rawIcon: iconWord));
    subMenu.addItem(NativeMenuItem.withRawIcon(title: "Excel", action: "action_excel", rawIcon: iconExcel));
    subMenu.addItem(NativeMenuItem.withRawIcon(title: "PDF", action: "action_pdf", rawIcon: iconPdf));
    itemNew.subMenu = subMenu;
    //显示本地的图片
    //注意：这个路径会根据沙箱原则确定是否会正常显示，请确保正常的可访问权限。
    String iconPath;
    if (Platform.isMacOS) {
      iconPath = "/Users/parcool/Downloads/paste.png";
    } else if (Platform.isLinux) {
      iconPath = "/home/parcool/Downloads/paste.png";
    } else if (Platform.isWindows) {
      iconPath = "C:\\Users\\parcool\\Downloads\\paste.png";
    } else {
      throw PlatformException(code: "Unsupported platform!");
    }
    NativeMenu menu = NativeMenu();
    menu.addItem(itemNew);
    menu.addItem(NativeMenuItem.simple(title: "New Folder", action: "action_new_folder"));
    menu.addItem(NativeMenuItem.separator());
    menu.addItem(NativeMenuItem.withIcon(title: "Paste", action: "action_paste", icon: iconPath, isEnable: false));
    return menu;
  }
```
2. 有两种方式
   1. 使用`NativeContextMenuWidget`：把你的Widget当作child放进去就可以了
        ```dart
          @override
          Widget build(BuildContext context) {
            return MaterialApp(
              home: Scaffold(
                appBar: AppBar(
                  title: const Text('Venyore Plugin example app'),
                ),
                body: FutureBuilder<NativeMenu>(
                  future: initMenu(),
                  builder: (BuildContext context, AsyncSnapshot<NativeMenu> snapshot) {
                    if (snapshot.hasData) {
                      return NativeContextMenuWidget(
                        actionCallback: (action) {
                          final actionString = action.toString();
                          // 自己根据actionString的值作后续操作
                          // switch (actionString) {
                          //   case "action...":
                          //     break;
                          // }
                        },
                        menu: snapshot.requireData,
                        otherCallback: (method) {
                          if (kDebugMode) {
                            print("$method called!");
                          }
                        },
                        child: const Text("Your own widget"),
                      );
                    } else if (snapshot.hasError) {
                      return const Text("Build NativeMenu get some error.");
                    } else {
                      return const CircularProgressIndicator();
                    }
                  },
                ),
              ),
            );
          }
        ```
   2. 直接使用`NativeContextMenuNg`：自己处理鼠标事件后，在你需要的时候弹出菜单
        ```dart
        void showNativeContextMenu() async {
          final nativeContextMenuNgPlugin = NativeContextMenuNg()
            ..setMethodCallHandler((call) async {
            switch (call.method) {
              case "onItemClicked":
                final arg = call.arguments as Map<dynamic, dynamic>;
                final actionString = arg["action"].toString();
                // switch (actionString) {
                //   case "action...":
                //     break;
                // }
                break;
              case "menuDidClose":
                if (kDebugMode) {
                    print("menuDidClose called!");
                  }
                break;
              default:
                break;
            }
          });
          await nativeContextMenuNgPlugin.showNativeContextMenu(widget.menu);
        }
        ```