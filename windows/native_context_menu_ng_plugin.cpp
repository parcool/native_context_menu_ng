#include "native_context_menu_ng_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <wincodec.h>
#include <Shlwapi.h>

#include <fstream>
#include <iterator>

#pragma comment(lib, "Shlwapi.lib")  // Link the Shlwapi library.
#pragma comment(lib, "Windowscodecs.lib") // Link the Windows Imaging Component (WIC) library.

#include <ShellScalingApi.h>

#pragma comment(lib, "Shcore.lib") // To achieve real-time DPI awareness.

using namespace std;
using namespace flutter;

namespace native_context_menu_ng {

    int32_t windowProcId = -1;
    flutter::PluginRegistrarWindows* reg;


    int id = 1;
    // Define the type of the container.
    typedef std::vector<HBITMAP> BitmapContainer;
    // Create a container.
    BitmapContainer bitmapContainer;
    HMENU mHMenu;


    std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel;
    
// static
void NativeContextMenuNgPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
    reg = std::move(registrar);
    channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "native_context_menu_ng",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<NativeContextMenuNgPlugin>();
  
  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

NativeContextMenuNgPlugin::NativeContextMenuNgPlugin() {
    windowProcId = reg->RegisterTopLevelWindowProcDelegate([this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        return this->HandleWindowProc(hwnd, message, wparam, lparam);
        });
}

NativeContextMenuNgPlugin::~NativeContextMenuNgPlugin() {
    reg->UnregisterTopLevelWindowProcDelegate(windowProcId);
}


/// <summary>
/// string => wstring
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
std::wstring NativeContextMenuNgPlugin::ConvertStringToWideString(const std::string& str)
{
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    std::wstring result(wideCharLength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], wideCharLength);
    return result;
}


/// <summary>
/// stringToLPCWSTR
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
LPCWSTR NativeContextMenuNgPlugin::stringToLPCWSTR(const std::string& str) {
    int length = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    wchar_t* buffer = new wchar_t[length];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, length);
    buffer[length - 1] = L'\0'; // Ensure that the buffer is terminated with the null character ('\0').
    return buffer;
}


/// <summary>
/// LPCWSTRToString
/// </summary>
/// <param name="wideStr"></param>
/// <returns></returns>
std::string NativeContextMenuNgPlugin::LPCWSTRToString(LPCWSTR wideStr)
{
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, NULL, 0, NULL, NULL);
    std::string str(bufferSize - 1, 0);//// Excluding the null terminator.
    WideCharToMultiByte(CP_UTF8, 0, wideStr, -1, &str[0], bufferSize, NULL, NULL);
    return str;
}


/// <summary>
/// Return the optimal icon size based on the screen scaling.
/// </summary>
/// <returns></returns>
int NativeContextMenuNgPlugin::getBestIconSize() {
    HWND hWnd = GetActiveWindow();
    HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
    UINT dpiX, dpiY;
    HRESULT hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    if (SUCCEEDED(hr)) {
        if (dpiX == 96) {
            return 16;
        }
        else if (dpiX == 120 || dpiX == 144) {
            return 24;
        }
        else {
            return 32;
        }
    }
    return 16;
}


/// <summary>
/// Load the PNG image of the raw data into an HBITMAP.
/// </summary>
/// <param name="pngBytes">Image data.</param>
/// <param name="hBitmap">The HBITMAP that has been created.</param>
/// <param name="width">The desired width after scaling.</param>
/// <param name="height">The desired height after scaling.</param>
/// <returns>HRESULT</returns>
HRESULT NativeContextMenuNgPlugin::CreateHBitmapFromPNGBytes(const std::vector<uint8_t>& pngBytes, HBITMAP& hBitmap, int width, int height) {
    HRESULT hr = S_OK;

    // Initialize the COM library.
    hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return hr;
    }

    IWICImagingFactory* pFactory = nullptr;
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICFormatConverter* pConverter = nullptr;
    IWICBitmapScaler* pScaler = nullptr;

    // Create a WIC factory.
    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        reinterpret_cast<void**>(&pFactory)
    );
    if (FAILED(hr)) {
        std::cerr << "Failed to create WIC imaging factory" << std::endl;
        CoUninitialize();
        return hr;
    }

    // Create a decoder.
    hr = pFactory->CreateDecoderFromStream(
        SHCreateMemStream(pngBytes.data(), static_cast<UINT>(pngBytes.size())),
        nullptr,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
    );
    if (FAILED(hr)) {
        std::cerr << "Failed to create WIC decoder for PNG image" << std::endl;
        pFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Retrieve the first frame of the image
    hr = pDecoder->GetFrame(0, &pFrame);
    if (FAILED(hr)) {
        std::cerr << "Failed to get frame from PNG image" << std::endl;
        pDecoder->Release();
        pFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Create a format converter
    hr = pFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr)) {
        std::cerr << "Failed to create WIC format converter" << std::endl;
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Initialize a format converter.
    hr = pConverter->Initialize(
        pFrame,
        GUID_WICPixelFormat32bppBGRA,  // The target pixel format, 32-bit BGRA.
        WICBitmapDitherTypeNone,
        nullptr,
        0.0,
        WICBitmapPaletteTypeCustom
    );
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize WIC format converter" << std::endl;
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Create a bitmap scaler
    hr = pFactory->CreateBitmapScaler(&pScaler);
    if (FAILED(hr)) {
        std::cerr << "Failed to create WIC bitmap scaler" << std::endl;
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Set the target size of the scaler
    hr = pScaler->Initialize(pConverter, width, height, WICBitmapInterpolationModeLinear);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize WIC bitmap scaler" << std::endl;
        pScaler->Release();
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        CoUninitialize();
        return hr;
    }

    // Retrieve the scaled image data.
    UINT scaledWidth = 0;
    UINT scaledHeight = 0;
    hr = pScaler->GetSize(&scaledWidth, &scaledHeight);
    if (FAILED(hr)) {
        std::cerr << "Failed to get scaled image size" << std::endl;
        pScaler->Release();
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pFactory->Release();
        CoUninitialize();
        return hr;
    }

    // create bitmap
    HDC hdc = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdc);
    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(scaledWidth);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(scaledHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    void* pBits = nullptr;
    HBITMAP hBitmapTemp = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
    ReleaseDC(nullptr, hdcMem);
    ReleaseDC(nullptr, hdc);
    DeleteDC(hdcMem);

    


    if (hBitmapTemp) {
        std::vector<uint8_t> tmpBits(scaledWidth * scaledHeight * 4);
        hr = pScaler->CopyPixels(nullptr, scaledWidth * 4, scaledWidth * scaledHeight * 4, tmpBits.data());
        if (FAILED(hr)) {
            std::cerr << "Failed to copy scaled pixel data to temporary buffer" << std::endl;
            SelectObject(hdcMem, hBitmapTemp);
            DeleteObject(hBitmapTemp);
        }
        else {
            // 预乘透明度。没有它的话图标会有毛刺，这个问题困扰了我好久好久。
            for (size_t i = 0; i < tmpBits.size(); i += 4) {
                float alpha = tmpBits[i + 3] / 255.0f;
                tmpBits[i] = static_cast<uint8_t>(tmpBits[i] * alpha); // Blue
                tmpBits[i + 1] = static_cast<uint8_t>(tmpBits[i + 1] * alpha); // Green
                tmpBits[i + 2] = static_cast<uint8_t>(tmpBits[i + 2] * alpha); // Red
            }
            memcpy(pBits, tmpBits.data(), tmpBits.size());

            pScaler->Release();
            pConverter->Release();
            pFrame->Release();
            pDecoder->Release();
            pFactory->Release();
            CoUninitialize();

            hBitmap = hBitmapTemp;
            return S_OK;
        }
    }
    else {
        std::cerr << "Failed to create DIB section" << std::endl;
    }

    pScaler->Release();
    pConverter->Release();
    pFrame->Release();
    pDecoder->Release();
    pFactory->Release();
    CoUninitialize();

    return E_FAIL;
}


/// <summary>
/// Load a PNG image from a local path as an HBITMAP
/// </summary>
/// <param name="filePath">image path.Argument type[std::string] </param>
/// <param name="width">Expected scaled width</param>
/// <param name="height">Expected scaled height</param>
/// <returns>built HBITMAP</returns>
HBITMAP NativeContextMenuNgPlugin::LoadPngImage(const std::string& filePath, int width, int height) {
    HBITMAP hBitmap = nullptr;

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return nullptr;
    }

    std::vector<uint8_t> pngBytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    HRESULT hr = CreateHBitmapFromPNGBytes(pngBytes, hBitmap, width, height);
    if (FAILED(hr)) {
        std::cerr << "Failed to load PNG image: " << filePath << std::endl;
        return nullptr;
    }

    return hBitmap;
}

/// <summary>
/// build menu data
/// </summary>
/// <param name="hMenu"></param>
/// <param name="items"></param>
void NativeContextMenuNgPlugin::buildPopupMenu(HMENU hMenu, std::vector<EncodableValue> items) {
    for (EncodableValue item : items) {
        EncodableMap map = std::get<EncodableMap>(item);
        std::map<EncodableValue, EncodableValue>::const_iterator typeIt = map.find(EncodableValue("type"));
        string type;
        if (typeIt != map.end()) {
            type = std::get<string>(typeIt->second);
        }
        if (type.compare("separator") == 0) {
            MENUITEMINFO menuItemInfo = {};
            menuItemInfo.cbSize = sizeof(MENUITEMINFO);
            menuItemInfo.fMask = MIIM_ID | MIIM_FTYPE;
            menuItemInfo.fType = MFT_SEPARATOR;
            menuItemInfo.wID = id++;
            InsertMenuItem(hMenu, menuItemInfo.wID, true, &menuItemInfo);
            cout << "added separator!" << endl;
        }
        else if (type.compare("menu") == 0) {
            //ref doc: https://learn.microsoft.com/zh-cn/windows/win32/api/winuser/ns-winuser-menuiteminfow
            MENUITEMINFO menuItemInfo = {};
            menuItemInfo.cbSize = sizeof(MENUITEMINFO);
            menuItemInfo.wID = id++;
            menuItemInfo.fMask = MIIM_ID | MIIM_STRING | MIIM_FTYPE | MIIM_STATE | MIIM_DATA | MIIM_SUBMENU;
            //title
            std::map<EncodableValue, EncodableValue>::const_iterator titleIt = map.find(EncodableValue("title"));
            string title2;
            if (titleIt != map.end()) {
                string title = std::get<string>(titleIt->second);
                title2 = title;
                size_t menuItemTextLength = strlen(title.c_str()) + 1;
                menuItemInfo.dwTypeData = const_cast<LPWSTR>(stringToLPCWSTR(title));
                menuItemInfo.cch = static_cast<UINT>(menuItemTextLength);
            }
            //subMenu
            flutter::EncodableMap::iterator subMenuIt = map.find(EncodableValue("subMenu"));
            EncodableMap subMenu;
            if (subMenuIt != map.end()) {
                flutter::EncodableMap subMenuMap;
                const flutter::EncodableValue& subMenuValue = subMenuIt->second;
                if (auto* subMenuMapPtr = std::get_if<flutter::EncodableMap>(&subMenuValue)) {
                    subMenu = *subMenuMapPtr;
                }
                else {
                    // Handle cases of type mismatch or null.
                    //cout << "[" << LPCWSTRToString(menuItemInfo.dwTypeData) << "] subMenu is null." << endl;
                }
            }

            if (!subMenu.empty()) {
                std::vector<EncodableValue> subitems = std::get<std::vector<EncodableValue>>(subMenu[flutter::EncodableValue("items")]);
                menuItemInfo.hSubMenu = CreatePopupMenu();
                buildPopupMenu(menuItemInfo.hSubMenu, subitems);
            }
            //isEnable
            std::map<EncodableValue, EncodableValue>::const_iterator isEnableIt = map.find(EncodableValue("isEnable"));
            if (isEnableIt != map.end()) {
                bool isEnable = std::get<bool>(isEnableIt->second);
                menuItemInfo.fState = isEnable ? MFS_ENABLED : MFS_DISABLED;
            }
            //icon
            string icon;
            std::map<EncodableValue, EncodableValue>::const_iterator iconIt = map.find(EncodableValue("icon"));
            if (iconIt != map.end()) {
                const std::string* iconValue = std::get_if<std::string>(&iconIt->second);
                if (iconValue != nullptr) {
                    icon = *iconValue;
                    menuItemInfo.fMask = menuItemInfo.fMask | MIIM_BITMAP;
                    //HBITMAP hBitmap = loadBmpFromPath(icon,32,32);
                    int bestIconSize = getBestIconSize();
                    HBITMAP hBitmap = LoadPngImage(icon.c_str(), bestIconSize, bestIconSize);
                    //add to contaier for delete after use.
                    bitmapContainer.push_back(hBitmap);
                    if (hBitmap == nullptr) {
                        cout << "Most likely, it is caused by a path or permission issue. A feasible path is: C:\\Users\\parcool\\test.png." << endl;
                    }
                    else {
                        menuItemInfo.hbmpItem = hBitmap;
                    }
                }
            }
            //rawIcon
            std::vector<uint8_t> rawIcon;
            if (icon.empty()) {
                std::map<EncodableValue, EncodableValue>::const_iterator rawIconIt = map.find(EncodableValue("rawIcon"));
                if (rawIconIt != map.end()) {
                    const std::vector<uint8_t>* rawIconValue = std::get_if<std::vector<uint8_t>>(&rawIconIt->second);
                    if (rawIconValue != nullptr) {
                        rawIcon = *rawIconValue;
                        int bestIconSize = getBestIconSize();
                        HBITMAP hBitmap = nullptr;
                        HRESULT hr = CreateHBitmapFromPNGBytes(rawIcon, hBitmap, bestIconSize, bestIconSize);
                        if (SUCCEEDED(hr)) {
                            menuItemInfo.fMask = menuItemInfo.fMask | MIIM_BITMAP;
                            if (hBitmap == nullptr) {
                                cout << "Failed to convert hBitmap." << endl;
                            }
                            else {
                                menuItemInfo.hbmpItem = hBitmap;
                            }
                        }
                        else {
                            std::cerr << "Failed to create HBITMAP from PNG image bytes" << std::endl;
                        }
                    }
                }
            }

            if (icon.empty() && rawIcon.empty()) {
                //cout << "[" << LPCWSTRToString(menuItemInfo.dwTypeData) << "] icon and rawIcon both empty." << endl;
            }
            //action
            std::string* action = new std::string();
            std::map<EncodableValue, EncodableValue>::const_iterator actionIt = map.find(EncodableValue("action"));
            if (actionIt != map.end()) {
                if (!actionIt->second.IsNull() && std::holds_alternative<std::string>(actionIt->second)) {
                    *action = std::get<std::string>(actionIt->second);
                    if (action != nullptr) {
                        menuItemInfo.dwItemData = reinterpret_cast<ULONG_PTR>(action);
                    }
                }
            }

            InsertMenuItem(hMenu, menuItemInfo.wID, true, &menuItemInfo);
        }
    }
}



void NativeContextMenuNgPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (method_call.method_name().compare("showNativeContextMenu") == 0) {
      auto* arguments = std::get_if<EncodableMap>(method_call.arguments());
      if (arguments) {
          std::map<EncodableValue, EncodableValue>::const_iterator itemsIt = arguments->find(EncodableValue("items"));
          if (itemsIt != arguments->end()) {
              EncodableValue itemsItEncodableValue = itemsIt->second;
              std::vector<EncodableValue> items = std::get<flutter::EncodableList>(itemsItEncodableValue);
              mHMenu = CreatePopupMenu();
              id = 1;//reset id;
              buildPopupMenu(mHMenu,items);
              HWND hWnd = GetActiveWindow();
              double pX{}, pY{};
              POINT cursor_pos;
              GetCursorPos(&cursor_pos);
              pX = cursor_pos.x;
              pY = cursor_pos.y;
              TrackPopupMenu(mHMenu, TPM_LEFTALIGN, static_cast<int>(pX), static_cast<int>(pY), 0, hWnd, nullptr);
          }
      }
    result->Success(flutter::EncodableValue(0));
  } else {
    result->NotImplemented();
  }
}


std::optional<LRESULT> NativeContextMenuNgPlugin::HandleWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int32_t clickedItem = static_cast<int32_t>(wparam);
        if (clickedItem != 0 && mHMenu) {
            MENUITEMINFO menuItemInfo = {};
            menuItemInfo.cbSize = sizeof(MENUITEMINFO);
            menuItemInfo.fMask = MIIM_ID | MIIM_DATA;
            if (GetMenuItemInfo(mHMenu, clickedItem, FALSE, &menuItemInfo)) {
                LPARAM itemData = menuItemInfo.dwItemData;
                std::string* customData = reinterpret_cast<std::string*>(itemData);
                if (customData != nullptr) {
                    flutter::EncodableMap encodableMap;
                    encodableMap[flutter::EncodableValue("action")] = flutter::EncodableValue(*customData);
                    flutter::EncodableValue encodableValue(encodableMap);
                    channel->InvokeMethod("onItemClicked", std::make_unique<flutter::EncodableValue>(encodableValue));
                    delete reinterpret_cast<std::string*>(itemData);
                }
            }
        }
    }
        break;
    case WM_EXITMENULOOP:
        for (HBITMAP hBitmap : bitmapContainer) {
            DeleteObject(hBitmap);
        }
        channel->InvokeMethod("menuDidClose", nullptr);
        break;
    case WM_DRAWITEM:
    {
        break;
    }
    default:
        break;
    }
    return std::nullopt;
}

}  // namespace native_context_menu_ng