# native_context_menu_ng
[[English]](README.md)  |  [[中文]](README_cn.md)

![Pub Version](https://img.shields.io/pub/v/native_context_menu_ng)

native_context_menu_ng is a Flutter plugin that provides native right-click menus for Flutter applications, supporting three major desktop platforms (macOS, Windows and Linux).

# Features
* Supports submenus;
* Supports icons (currently only supports PNG format for platform consistency);
* Icon size doesn't need to be specified manually. it automatically adjusts to the optimal size based on platform hardware and scaling settings;
* Supports separators;
* Supports disabling/enabling menu items."

<img src="https://github.com/parcool/native_context_menu_ng/blob/main/screenshot.gif?raw=true" alt="gif" />

# Usage
1. Build the menu first.
```dart
Future<NativeMenu> initMenu() async {
    //It has no action if it has submenu.
    NativeMenuItem itemNew = NativeMenuItem.simple(title: "New");
    //image from asset
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
    //image from local path
    //please note that the local path icon has limitations within the app sandbox.
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
2. There are two ways.
    1. Using `NativeContextMenuWidget`: Simply put your widget as a child inside it.
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
    2. Directly using `NativeContextMenuNg`: Handle mouse events yourself and pop up the menu when needed.
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