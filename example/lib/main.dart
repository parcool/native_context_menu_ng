import 'dart:io';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:native_context_menu_ng/native_context_menu_widget.dart';
import 'package:native_context_menu_ng/native_menu.dart';
import 'package:native_context_menu_ng/venyore_image_util.dart';
import 'package:image/image.dart' as img;

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {

  Future<Uint8List> convertBMPtoUint8List(String assetPath, {int? width, int? height}) async {
    final ByteData byteData = await rootBundle.load(assetPath);
    final Uint8List bmpBytes = byteData.buffer.asUint8List();

    if (width != null && height != null) {
      final img.Image? bmpImage = img.decodeImage(bmpBytes);
      if (bmpImage != null) {
        final img.Image bmpImageResized = img.copyResize(bmpImage, width: width, height: height);

        return Future.value(Uint8List.fromList(img.encodePng(bmpImageResized)));
      }
    }
    return Future.value(bmpBytes);
  }

  String? _clickedAction;

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
                  setState(() {
                    _clickedAction = actionString;
                  });
                  // switch (actionString) {
                  //   case "action1":
                  //     break;
                  //   case "action2":
                  //     break;
                  //   default:
                  //     break;
                  // }
                },
                menu: snapshot.requireData,
                otherCallback: (method) {
                  if (kDebugMode) {
                    print("$method called!");
                  }
                },
                child: Container(
                  color: Colors.grey[350],
                  child: Column(
                    children: [
                      SizedBox(
                          height: 100,
                          child: Align(
                            alignment: Alignment.bottomCenter,
                            child: Text(
                              "Clicked menu action : $_clickedAction",
                              style: TextStyle(color: Colors.green[600]),
                            ),
                          )),
                      Expanded(
                        child: Center(
                          child: Text('Right click anywhere.', style: TextStyle(color: Colors.grey[500])),
                        ),
                      ),
                    ],
                  ),
                ),
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
}
