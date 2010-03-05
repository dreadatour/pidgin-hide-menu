/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 
    ---
    Copyright (C) 2010, Vladimir Rudnyh <mail@dreadatour.ru>
*/

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gtkutils.h>
#include <string.h>

#include <conversation.h>
#include <plugin.h>
#include <signals.h>
#include <util.h>
#include <version.h>

#include <gtkblist.h>
#include <gtkconv.h>
#include <gtkplugin.h>

#include <gtk/gtkwindow.h>
#include <gdk/gdkwindow.h>

// current menu visibility state
gboolean is_menu_visible = TRUE;

/**************************************************************************************************
    Toggle menubars visibility
**************************************************************************************************/
static void
toggle_menubar(PidginBuddyList *gtkblist, PidginConversation *gtkconv)
{
    GtkWidget *menu = NULL;

    // get blist menu
    if (gtkblist != NULL) {
        menu = gtk_item_factory_get_widget(gtkblist->ift, "<PurpleMain>");
    }

    if (is_menu_visible) {
        // show all elements
        if (gtkblist != NULL && purple_prefs_get_bool("/plugins/core/hidemenu/menu_blist") && !gtk_widget_get_visible(menu)) {
            gtk_widget_show(menu);
        }
        if (gtkblist != NULL && purple_prefs_get_bool("/plugins/core/hidemenu/status_blist") && !gtk_widget_get_visible(gtkblist->statusbox)) {
            gtk_widget_show(gtkblist->statusbox);
        }
        if (gtkconv != NULL && purple_prefs_get_bool("/plugins/core/hidemenu/menu_conv") && !gtk_widget_get_visible(gtkconv->win->menu.menubar)) {
            gtk_widget_show(gtkconv->win->menu.menubar);
        }
    } else {
        // hide all elements
        if (gtkblist != NULL && purple_prefs_get_bool("/plugins/core/hidemenu/menu_blist") && gtk_widget_get_visible(menu)) {
            gtk_widget_hide(menu);
        }
        if (gtkblist != NULL && purple_prefs_get_bool("/plugins/core/hidemenu/status_blist") && gtk_widget_get_visible(gtkblist->statusbox)) {
            gtk_widget_hide(gtkblist->statusbox);
        }
        if (gtkconv != NULL && purple_prefs_get_bool("/plugins/core/hidemenu/menu_conv") && gtk_widget_get_visible(gtkconv->win->menu.menubar)) {
            gtk_widget_hide(gtkconv->win->menu.menubar);
        }
    }
}

/**************************************************************************************************
    Toggle menubars state
**************************************************************************************************/
static void toggle_menubar_state()
{
    PidginBuddyList *gtkblist = pidgin_blist_get_default_gtk_blist();
	GList *convs = purple_get_conversations();
   
    // toggle menu visibility status
    is_menu_visible = !is_menu_visible;
    
    // toggle blist
    toggle_menubar(gtkblist, NULL);
	
    // toggle conversations
    while (convs) {
		PurpleConversation *conv = (PurpleConversation *)convs->data;
		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			toggle_menubar(NULL, PIDGIN_CONVERSATION(conv));
		}
		convs = convs->next;
	}
}

/**************************************************************************************************
    Key was pressed
**************************************************************************************************/
static GdkFilterReturn 
key_press(GdkXEvent *EventData, GdkEvent *Event, gpointer Data) 
{
    // make sure this is a key event
    if(((XEvent*)EventData)->type != KeyPress) {
        return GDK_FILTER_CONTINUE;
    }

    // extract the key modifiers
    XKeyEvent *KeyEvent=(XKeyEvent*)EventData;

    // if key 'F12' pressed - toggle menus
    if (KeyEvent->keycode == 96) {
        toggle_menubar_state();
        return GDK_FILTER_REMOVE;
    }  

    return GDK_FILTER_CONTINUE;
}

/**************************************************************************************************
    Buddy list was created
**************************************************************************************************/
static void 
blist_created_cb(PurpleBuddyList *blist)
{
    PidginBuddyList *gtkblist = PIDGIN_BLIST(blist);
    GdkWindow *root = gtk_widget_get_toplevel(GTK_WIDGET(gtkblist->window))->window;

    // set keygrabber if needed
    if (g_object_get_data((GObject *) root, "filter_set") == NULL) {
        gdk_window_add_filter(root, key_press, 0);
        g_object_set_data((GObject *) root, "filter_set", "1");
    }
	
    // hide menubar
    toggle_menubar(gtkblist, NULL);
}

/**************************************************************************************************
    Conversation was displayed
**************************************************************************************************/
static void
conversation_displayed_cb(PidginConversation *gtkconv)
{
    PidginBuddyList *gtkblist = pidgin_blist_get_default_gtk_blist();
    GdkWindow *root = gtk_widget_get_toplevel(GTK_WIDGET(gtkconv->win->window))->window;
    
    // set keygrabber if needed
    if (g_object_get_data((GObject *) root, "filter_set") == NULL) {
        gdk_window_add_filter(root, key_press, 0);
        g_object_set_data((GObject *) root, "filter_set", "1");
    }

    // hide menubar
	toggle_menubar(gtkblist, gtkconv);
}

/**************************************************************************************************
    Load plugin
**************************************************************************************************/
static gboolean
plugin_load(PurplePlugin *plugin)
{
    PidginBuddyList *gtkblist = pidgin_blist_get_default_gtk_blist();
	GList *convs = purple_get_conversations();

    if (purple_prefs_get_bool("/plugins/core/hidemenu/default_state")) {
        is_menu_visible = FALSE;
    } else {
        is_menu_visible = TRUE;
    }

    // set callback for 'blist created' signal
    purple_signal_connect(pidgin_blist_get_handle(), "gtkblist-created", plugin, 
                          PURPLE_CALLBACK(blist_created_cb), NULL);
    // set callback for 'conversation displayed' signal
	purple_signal_connect(pidgin_conversations_get_handle(), "conversation-displayed", plugin,
	                      PURPLE_CALLBACK(conversation_displayed_cb), NULL);

	// hide blist menubar
    toggle_menubar(gtkblist, NULL);

    // hide conversations menubar
	while (convs) {
		PurpleConversation *conv = (PurpleConversation *)convs->data;
		if (PIDGIN_IS_PIDGIN_CONVERSATION(conv)) {
			toggle_menubar(NULL, PIDGIN_CONVERSATION(conv));
		}
		convs = convs->next;
	}

	return TRUE;
}

static PurplePluginPrefFrame *
get_plugin_pref_frame(PurplePlugin *plugin) {
    PurplePluginPrefFrame *frame;
    PurplePluginPref *ppref;

    frame = purple_plugin_pref_frame_new();

    ppref = purple_plugin_pref_new_with_name_and_label(
        "/plugins/core/hidemenu/default_state", "Hide menus on start");
        purple_plugin_pref_frame_add(frame, ppref);

    ppref = purple_plugin_pref_new_with_name_and_label(
        "/plugins/core/hidemenu/menu_blist", "Toggle buddy list menu");
        purple_plugin_pref_frame_add(frame, ppref);

    ppref = purple_plugin_pref_new_with_name_and_label(
        "/plugins/core/hidemenu/status_blist", "Toggle buddy list status box");
        purple_plugin_pref_frame_add(frame, ppref);

    ppref = purple_plugin_pref_new_with_name_and_label(
        "/plugins/core/hidemenu/menu_conv", "Toggle conversations menu");
        purple_plugin_pref_frame_add(frame, ppref);

    return frame;
}


/**************************************************************************************************
    Info about preferences
**************************************************************************************************/
static PurplePluginUiInfo prefs_info = { 
    get_plugin_pref_frame,
    0,                      // page_num (reserved)
    NULL,                   // frame (reserved)
    NULL,
    NULL,
    NULL,
    NULL
};


/**************************************************************************************************
    Info about plugin
**************************************************************************************************/
static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,                                        // magic
    PURPLE_MAJOR_VERSION,                                       // major version
    PURPLE_MINOR_VERSION,                                       // minor version
    PURPLE_PLUGIN_STANDARD,                                     // type
    NULL,                                                       // ui_requirement
    0,                                                          // flags
    NULL,                                                       // dependencies
    PURPLE_PRIORITY_DEFAULT,                                    // priority
    "core-plugin-hidemenu",                                     // id
    "Hide Menu",                                                // name
    "0.1",                                                      // version
	"Hide menubar",                                             // summary
	"Hide menubar in conversation and contact list window.",    // description
    "Vladimir Rudnyh <dreadatour@gmail.com>",                   // author
    "http://dreadatour.ru/",                                    // homepage
    plugin_load,                                                // load
    NULL,                                                       // unload
    NULL,                                                       // destroy
    NULL,                                                       // ui_info
    NULL,                                                       // extra_info
    &prefs_info,                                                // prefs_info
    NULL,                                                       // actions
    NULL,
    NULL,
    NULL,
    NULL
};

/**************************************************************************************************
    Initialize plugin
**************************************************************************************************/
static void
init_plugin(PurplePlugin *plugin)
{
    purple_prefs_add_none("/plugins/core/hidemenu");
    purple_prefs_add_bool("/plugins/core/hidemenu/default_state", TRUE);
    purple_prefs_add_bool("/plugins/core/hidemenu/menu_blist", TRUE);
    purple_prefs_add_bool("/plugins/core/hidemenu/status_blist", TRUE);
    purple_prefs_add_bool("/plugins/core/hidemenu/menu_conv", TRUE);
}

//  and now we will go and initialize our plugin
PURPLE_INIT_PLUGIN(hidemenu, init_plugin, info)

