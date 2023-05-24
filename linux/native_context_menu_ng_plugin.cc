#include "include/native_context_menu_ng/native_context_menu_ng_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>

#define NATIVE_CONTEXT_MENU_NG_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), native_context_menu_ng_plugin_get_type(), \
                              NativeContextMenuNgPlugin))

struct _NativeContextMenuNgPlugin {
    GObject parent_instance;
    FlPluginRegistrar *registrar;
};


G_DEFINE_TYPE(NativeContextMenuNgPlugin, native_context_menu_ng_plugin, g_object_get_type())

struct SignalData {
    FlMethodChannel *channel;
    const char *action;
};

///////////////////////////////menu-method-start////////////////////////////////////
static void menu_item_callback(GtkWidget *widget, gpointer data) {
    auto *signalData = (SignalData *) data;
    FlMethodChannel *channel = signalData->channel;
    const char *action = signalData->action;
    if (action) {
        g_autoptr(FlValue) map = fl_value_new_map();
        fl_value_set_string(map, "action", fl_value_new_string(action));
        fl_method_channel_invoke_method(channel, "onItemClicked", map, nullptr, nullptr, nullptr);
    }
    delete[] signalData->action;
    delete signalData;
}

// 菜单关闭事件回调函数
void on_menu_closed(GtkWidget *menu, gpointer data) {
    auto *signalData = (SignalData *) data;
    FlMethodChannel *channel = signalData->channel;
    fl_method_channel_invoke_method(channel, "menuDidClose", nullptr, nullptr, nullptr, nullptr);
    delete signalData;
}

GtkWidget *create_image_from_uint8list(const uint8_t *rawImageData, size_t length) {
    if (rawImageData == nullptr) {
        g_print("Error: Invalid raw image data.\n");
        return nullptr;
    }
    GdkPixbufLoader* loader = gdk_pixbuf_loader_new();
    if (!loader) {
        g_print("Error: Failed to create GdkPixbufLoader.\n");
        return nullptr;
    }
    GError* error = nullptr;
    gboolean success = gdk_pixbuf_loader_write(loader, rawImageData, length, &error);
    //这个close的小问题也耽误了不少时间：https://gitlab.gnome.org/GNOME/gdk-pixbuf/-/issues/110
    gdk_pixbuf_loader_close(loader, nullptr);
    if (!success) {
        g_print("Error: Failed to write image data to loader: %s\n", error->message);
        g_error_free(error);
        g_object_unref(loader);
        return nullptr;
    }
    GdkPixbuf *pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
    if (pixbuf == nullptr) {
        g_print("Error: Failed to create GdkPixbuf from image data\n");
        g_object_unref(loader);
        return nullptr;
    }

    // 再次检查 pixbuf 是否有效
    if (!GDK_IS_PIXBUF(pixbuf)) {
        g_print("Error: Invalid GdkPixbuf object.\n");
        g_object_unref(pixbuf);
        g_object_unref(loader);
        return nullptr;
    }

    // 处理透明通道premultiply alpha
    if (gdk_pixbuf_get_has_alpha(pixbuf)) {
        guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
        int row_stride_temp = gdk_pixbuf_get_rowstride(pixbuf);
        int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
        for (int y = 0; y < gdk_pixbuf_get_height(pixbuf); y++) {
            for (int x = 0; x < gdk_pixbuf_get_width(pixbuf); x++) {
                int offset = y * row_stride_temp + x * n_channels;
                float alpha = pixels[offset + 3] / 255.0f;
                pixels[offset] = pixels[offset] * alpha;
                pixels[offset + 1] = pixels[offset + 1] * alpha;
                pixels[offset + 2] = pixels[offset + 2] * alpha;
            }
        }
    }
    GtkWidget* image = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);
    return image;
}

GtkWidget *create_resized_image(GtkWidget *image, gint new_width, gint new_height) {
    GdkPixbuf *pixbuf = gtk_image_get_pixbuf(GTK_IMAGE(image));
    GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(pixbuf, new_width, new_height, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(image), scaled_pixbuf);
    g_object_unref(scaled_pixbuf);
    return image;
}

GtkWidget *create_resized_image_with_path(const gchar *image_path, gint new_width, gint new_height) {
    GtkWidget *image = gtk_image_new_from_file(image_path);
    return create_resized_image(image, new_width, new_height);
}

GtkWidget *create_resized_image_with_raw(const uint8_t *rawImageData, size_t length, gint new_width, gint new_height) {
    GtkWidget *image = create_image_from_uint8list(rawImageData, length);
    if (image == nullptr) {
        return nullptr;
    }
    return create_resized_image(image, new_width, new_height);
}

GtkWidget *createMenuItemSeparator() {
    return gtk_separator_menu_item_new();
}

GtkWidget *create_menu_item_with_label(FlMethodChannel *channel, const gchar *label, const char *action, bool isEnable) {
    GtkWidget *menu_item = gtk_menu_item_new_with_label(label);
    gtk_widget_set_sensitive(GTK_WIDGET(menu_item), isEnable ? TRUE : FALSE);
    if (action) {
        auto *signalData = new SignalData();
        signalData->channel = channel;
        signalData->action = g_strdup(action);
        g_signal_connect(menu_item, "activate", G_CALLBACK(menu_item_callback), signalData);
    }
    return menu_item;
}

GtkWidget *
create_menu_item_with_icon_and_label(FlMethodChannel *channel, GtkWidget *icon, const gchar *label, const char *action, bool isEnable) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), create_menu_item_with_label(channel, label, nullptr, isEnable), FALSE, FALSE, 0);

    GtkWidget *menu_item = gtk_menu_item_new();
    // 设置菜单项为启用状态
    gtk_widget_set_sensitive(GTK_WIDGET(menu_item), isEnable ? TRUE : FALSE);

    if (action) {
        auto *signalData = new SignalData();
        signalData->channel = channel;
        signalData->action = g_strdup(action);
        g_signal_connect(menu_item, "activate", G_CALLBACK(menu_item_callback), signalData);
    }
    gtk_container_add(GTK_CONTAINER(menu_item), box);
    return menu_item;
}

GtkWidget *
create_menu_item_with_icon_and_label(FlMethodChannel *channel, const gchar *image_path, gint new_width, gint new_height, const gchar *label, const char *action, bool isEnable) {
    GtkWidget *icon = create_resized_image_with_path(image_path, new_width, new_height);
    return create_menu_item_with_icon_and_label(channel, icon, label, action, isEnable);
}

GtkWidget *
create_menu_item_with_raw_icon_and_label(FlMethodChannel *channel, const uint8_t *rawImageData, size_t length, gint new_width, gint new_height, const gchar *label, const char *action, bool isEnable) {
    GtkWidget *icon = create_resized_image_with_raw(rawImageData, length, new_width, new_height);
    return create_menu_item_with_icon_and_label(channel, icon, label, action, isEnable);
}

void build_menu(_FlValue *itemsFL, GtkWidget *menu, FlMethodChannel *channel, gint new_width, gint new_height) {
    if (fl_value_get_type(itemsFL) != FL_VALUE_TYPE_NULL) {
        for (int i = 0; i < fl_value_get_length(itemsFL); i++) {
            auto item = fl_value_get_list_value(itemsFL, i);
            const char *type = fl_value_get_string(fl_value_lookup_string(item, "type"));
            if (strcmp(type, "separator") == 0) {
                gtk_menu_shell_append(GTK_MENU_SHELL(menu), createMenuItemSeparator());
            } else if (strcmp(type, "menu") == 0) {
                const char *title = fl_value_get_string(fl_value_lookup_string(item, "title"));
                bool isEnable = fl_value_get_bool(fl_value_lookup_string(item, "isEnable"));
                const char *icon = nullptr;
                const uint8_t *rawIconData = nullptr;
                size_t length = 0;

                auto iconFL = fl_value_lookup_string(item, "icon");
                if (fl_value_get_type(iconFL) != FL_VALUE_TYPE_NULL) {
                    icon = fl_value_get_string(iconFL);
                } else {
                    auto rawIconFL = fl_value_lookup_string(item, "rawIcon");
                    if (fl_value_get_type(rawIconFL) == FL_VALUE_TYPE_UINT8_LIST) {
                        length = fl_value_get_length(rawIconFL);
                        rawIconData = fl_value_get_uint8_list(rawIconFL);
                    }
                }
                auto subMenuFL = fl_value_lookup_string(item, "subMenu");
                if (fl_value_get_type(subMenuFL) != FL_VALUE_TYPE_NULL) {
                    GtkWidget *subMenu = gtk_menu_new();
                    auto sbuItemsFL = fl_value_lookup_string(subMenuFL, "items");
                    build_menu(sbuItemsFL, subMenu, channel, new_width, new_height);

                    GtkWidget *menuItem;
                    if (icon) {
                        menuItem = create_menu_item_with_icon_and_label(channel, icon, new_width, new_height, title, nullptr, isEnable);
                    } else if (rawIconData) {
                        menuItem = create_menu_item_with_raw_icon_and_label(channel, rawIconData, length, new_width, new_height, title, nullptr, isEnable);
                    } else {
                        menuItem = create_menu_item_with_label(channel, title, nullptr, isEnable);
                    }
                    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);
                    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);

                } else {
                    auto actionFL = fl_value_lookup_string(item, "action");
                    if (fl_value_get_type(actionFL) != FL_VALUE_TYPE_NULL) {
                        const char *action = fl_value_get_string(actionFL);
                        GtkWidget *menuItem;
                        if (icon) {
                            menuItem = create_menu_item_with_icon_and_label(channel, icon, new_width, new_height, title, action, isEnable);
                        } else if (rawIconData) {
                            menuItem = create_menu_item_with_raw_icon_and_label(channel, rawIconData, length, new_width, new_height, title, action, isEnable);
                        } else {
                            menuItem = create_menu_item_with_label(channel, title, action, isEnable);
                        }
                        gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
                    } else {
                        g_printerr("It no subMenu and no action. Is something get wrong?");
                    }
                }
            }
        }
    }
}

void
right_click_menu_callback(GdkWindow *window, FlMethodChannel *channel, _FlValue *itemsFL) {
    //显示菜单
    GdkRectangle rectangle;
    GdkDevice *mouse_device;
    GdkSeat *seat = gdk_display_get_default_seat(gdk_display_get_default());
    mouse_device = gdk_seat_get_pointer(seat);
    gdk_window_get_device_position(window, mouse_device, &rectangle.x, &rectangle.y, nullptr);

    //determine the icon's size.
    GtkIconSize size = GTK_ICON_SIZE_MENU;
    gint width, height;
    gtk_icon_size_lookup(size, &width, &height);
    gdouble my_scale_factor = 1.5f;// I can't get the scale using api. I don't know why it always returns 1. but 1.5f seems good.
    width = static_cast<gint>(width * my_scale_factor);
    height = static_cast<gint>(height * my_scale_factor);

    // menu
    GtkWidget *menu = gtk_menu_new();
    build_menu(itemsFL, menu, channel, width, height);

    gtk_widget_show_all(menu);

    GdkEventButton event = {};
    GdkDisplay *display = gdk_display_get_default();
    event.window = window;
    event.device = gdk_seat_get_pointer(gdk_display_get_default_seat(display));

    gtk_menu_popup_at_rect(GTK_MENU(menu), window, &rectangle,
                           GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_NORTH_WEST,
                           (GdkEvent *) &event);

    auto *signalData = new SignalData();
    signalData->channel = channel;
    g_signal_connect(menu, "hide", G_CALLBACK(on_menu_closed), signalData);
}



///////////////////////////////menu-method-end////////////////////////////////////

static inline GdkWindow *get_window(NativeContextMenuNgPlugin *self) {
    FlView *view = fl_plugin_registrar_get_view(self->registrar);
    if (view == nullptr) return nullptr;
    return gtk_widget_get_window(gtk_widget_get_toplevel(GTK_WIDGET(view)));
}

// Called when a method call is received from Flutter.
static void native_context_menu_ng_plugin_handle_method_call(
        FlMethodChannel *channel,
        NativeContextMenuNgPlugin *self,
        FlMethodCall *method_call) {
    g_autoptr(FlMethodResponse) response = nullptr;

    const gchar *method = fl_method_call_get_name(method_call);

    if (strcmp(method, "showNativeContextMenu") == 0) {
        auto arguments = fl_method_call_get_args(method_call);
        //auto positionFL = fl_value_lookup_string(arguments, "position"); //useless
        auto itemsFL = fl_value_lookup_string(arguments, "items");

        GdkWindow *window = get_window(self);
        right_click_menu_callback(window, channel, itemsFL);

        g_autoptr(FlValue) ok = fl_value_new_int(0);
        response = FL_METHOD_RESPONSE(fl_method_success_response_new(ok));
    } else {
        response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
    }

    fl_method_call_respond(method_call, response, nullptr);
}

static void native_context_menu_ng_plugin_dispose(GObject *object) {
    G_OBJECT_CLASS(native_context_menu_ng_plugin_parent_class)->dispose(object);
}

static void native_context_menu_ng_plugin_class_init(NativeContextMenuNgPluginClass *klass) {
    G_OBJECT_CLASS(klass)->dispose = native_context_menu_ng_plugin_dispose;
}

static void native_context_menu_ng_plugin_init(NativeContextMenuNgPlugin *self) {}

static void method_call_cb(FlMethodChannel *channel, FlMethodCall *method_call,
                           gpointer user_data) {
    NativeContextMenuNgPlugin *plugin = NATIVE_CONTEXT_MENU_NG_PLUGIN(user_data);
    native_context_menu_ng_plugin_handle_method_call(channel, plugin, method_call);
}

void native_context_menu_ng_plugin_register_with_registrar(FlPluginRegistrar *registrar) {
    NativeContextMenuNgPlugin *plugin = NATIVE_CONTEXT_MENU_NG_PLUGIN(
            g_object_new(native_context_menu_ng_plugin_get_type(), nullptr));

    plugin->registrar = FL_PLUGIN_REGISTRAR(g_object_ref(registrar));

    g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
    g_autoptr(FlMethodChannel) channel =
            fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                                  "native_context_menu_ng",
                                  FL_METHOD_CODEC(codec));

    fl_method_channel_set_method_call_handler(channel, method_call_cb,
                                              g_object_ref(plugin),
                                              g_object_unref);

    g_object_unref(plugin);
}
