import 'dart:io';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';


class VenyoreImageUtil {
  static Future<Uint8List> iconDataToUint8List(IconData iconData, {double size = 100.0, Color color = Colors.black}) async {
    final ui.PictureRecorder pictureRecorder = ui.PictureRecorder();
    final Canvas canvas = Canvas(pictureRecorder);
    final Paint paint = Paint()..color = color;
    final ui.Path path = ui.Path();

    path.addPolygon([
      const Offset(0, 0),
      Offset(size, 0),
      Offset(size, size),
      Offset(0, size),
    ], true);

    canvas.drawPath(
        path.shift(Offset((size - iconData.fontPackage!.length) / 2, (size - iconData.codePoint) / 2)),
        paint);

    final ui.Image image = await pictureRecorder.endRecording().toImage(size.toInt(), size.toInt());
    final ByteData? byteData = await image.toByteData(format: ui.ImageByteFormat.png);

    return byteData!.buffer.asUint8List();
  }


  static Future<Uint8List> imageFileToUint8List(String imagePath) async {
    File imageFile = File(imagePath);
    Uint8List uint8list = await imageFile.readAsBytes();
    return uint8list;
  }

  static Future<Uint8List> assetImageToUint8List(String assetPath, {double width = 32, double height = 32}) async {
    final ByteData byteData = await rootBundle.load(assetPath);
    final ui.Codec codec = await ui.instantiateImageCodec(byteData.buffer.asUint8List(), targetWidth: width.toInt(), targetHeight: height.toInt());
    final ui.FrameInfo frameInfo = await codec.getNextFrame();

    final ByteData? resizedByteData = await frameInfo.image.toByteData(format: ui.ImageByteFormat.png);

    return resizedByteData!.buffer.asUint8List();
  }

}