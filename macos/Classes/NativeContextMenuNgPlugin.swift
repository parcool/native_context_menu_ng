import Cocoa
import FlutterMacOS

//official docs:
//NSMenu: https://developer.apple.com/documentation/appkit/nsmenu
//NSMenuItem: https://developer.apple.com/documentation/appkit/nsmenuitem

public class NativeContextMenuNgPlugin: NSObject, FlutterPlugin, NSMenuDelegate {

    var channel:FlutterMethodChannel?
    
    public static func register(with registrar: FlutterPluginRegistrar) {
        let channel = FlutterMethodChannel(name: "native_context_menu_ng", binaryMessenger: registrar.messenger)
        let instance = NativeContextMenuNgPlugin()
        instance.channel = channel
        registrar.addMethodCallDelegate(instance, channel: channel)
    }
    
    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
        case "showNativeContextMenu":
            guard let menu = call.arguments as? [String: Any] else {
                return
            }
            guard let items = menu["items"] as? [[String: Any]] else {
                return
            }
            let nsMenu = NSMenu()
            nsMenu.delegate = self
            buildNSMenu(menu: nsMenu, items: items)

            if let mainWindow = NSApplication.shared.windows.first {
                let mouseLocation = NSEvent.mouseLocation
                
                if let contentView = mainWindow.contentView,
                   let windowFrame = contentView.window?.frame {
                    let convertedOrigin = contentView.convert(NSPoint.zero, to: nil)
                    let viewLocation = NSPoint(x: mouseLocation.x - windowFrame.origin.x - convertedOrigin.x,
                                               y: mouseLocation.y - windowFrame.origin.y - convertedOrigin.y)
                    
                    nsMenu.popUp(positioning: nil, at: viewLocation, in: contentView)
                }
            }
            result(0)
        default:
            result(FlutterMethodNotImplemented)
        }
    }
    
    @objc public func menuItem1Clicked(_ sender: NSMenuItem) {
        if let action = sender.representedObject as? String {
            channel?.invokeMethod("onItemClicked", arguments: ["action": action])
        }
    }
    
    public func menuDidClose(_ menu: NSMenu) {
        channel?.invokeMethod("menuDidClose", arguments: nil)
    }
    
    private func buildNSMenu(menu: NSMenu, items: [[String: Any]]) {
        for item in items {
            guard let type = item["type"] as? String else {
                return
            }
            if type == "separator" {
                menu.addItem(NSMenuItem.separator())
            } else if type == "menu" {
                let menuItem = NSMenuItem()
                if let title = item["title"] as? String {
                    menuItem.title = title
                }
                
                if let subMenuData = item["subMenu"] as? [String: Any], let subItems = subMenuData["items"] as? [[String: Any]] {
                    if !subItems.isEmpty {
                        menuItem.submenu = NSMenu()
                        buildNSMenu(menu: menuItem.submenu!, items: subItems)
                    }
                }
                if let isEnable = item["isEnable"] as? Bool {
                    menuItem.isEnabled = isEnable
                }
                
                if let action = item["action"] as? String {
                    menuItem.representedObject = action
                    if !menuItem.hasSubmenu && menuItem.isEnabled {
                        menuItem.target = self
                        menuItem.action = #selector(menuItem1Clicked(_:))
                    }
                }
                
                if let icon = item["icon"] as? String {
                    let iconUrl = URL(fileURLWithPath: icon)
                    if let imageIcon = NSImage(contentsOf: iconUrl) {
                        imageIcon.size = NSSize(width: 18, height: 18)
                        menuItem.image = imageIcon
                    } else {
                        let message = "Failed to create NSImage from data. Do you get the file access permission OR does it exist? The icon path is: \(icon)"
                        NSLog("%@", message)
                    }
                } else if let rawIcon = item["rawIcon"] as? FlutterStandardTypedData, let imageIcon = NSImage(data: rawIcon.data) {
                    imageIcon.size = NSSize(width: 18, height: 18)
                    menuItem.image = imageIcon
                }
                
                menu.addItem(menuItem)
            }
        }
    }
}
