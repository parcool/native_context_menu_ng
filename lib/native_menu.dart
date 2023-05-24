import 'dart:typed_data';

enum MenuItemType { separator, menu }

class NativeMenu {
  final List<NativeMenuItem> items = [];

  void addItem(NativeMenuItem item) {
    items.add(item);
  }

  bool insertItem(int index, NativeMenuItem item) {
    if (index < 0 || index > items.length) {
      return false;
    }
    items.insert(index, item);
    return true;
  }

  bool removeItem(int index) {
    if (index < 0 || index >= items.length) {
      return false;
    }
    items.removeAt(index);
    return true;
  }

  void clearItem() {
    items.clear();
  }

  Map<String, dynamic> toJson() => {
        'items': items.map((item) => item.toJson()).toList(),
      };
}

class NativeMenuItem {
  final MenuItemType type;
  final String title;
  final String? action;
  final bool isEnable;
  final String? icon;
  final Uint8List? rawIcon;
  NativeMenu? subMenu;

  NativeMenuItem.separator()
      : type = MenuItemType.separator,
        title = "",
        action = "",
        isEnable = false,
        icon = null,
        rawIcon = null,
        subMenu = null;

  NativeMenuItem.simple({required this.title, this.action, this.isEnable = true, this.subMenu})
      : type = MenuItemType.menu,
        icon = null,
        rawIcon = null;

  NativeMenuItem.withIcon({required this.title, this.action, this.isEnable = true, required this.icon, this.subMenu})
      : type = MenuItemType.menu,
        rawIcon = null;

  NativeMenuItem.withRawIcon({required this.title, this.action, this.isEnable = true, required this.rawIcon, this.subMenu})
      : type = MenuItemType.menu,
        icon = null;

  void setSubMenu(NativeMenu subMenu) {
    this.subMenu = subMenu;
  }

  Map<String, dynamic> toJson() => {
        'type': type.name,
        'title': title,
        'action': action,
        'isEnable': isEnable,
        'icon': icon,
        'rawIcon': rawIcon,
        'subMenu': subMenu?.toJson(),
      };
}
