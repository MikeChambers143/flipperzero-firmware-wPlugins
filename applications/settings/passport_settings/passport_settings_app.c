#include <furi.h>
#include <gui/scene_manager.h>

#include "passport_settings_app.h"
#include "scenes/passport_settings_scene.h"

static bool passport_settings_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    PassportSettingsApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool passport_settings_back_event_callback(void* context) {
    furi_assert(context);
    PassportSettingsApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

PassportSettingsApp* passport_settings_app_alloc() {
    PassportSettingsApp* app = malloc(sizeof(PassportSettingsApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&passport_settings_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, passport_settings_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, passport_settings_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    app->variable_item_list = variable_item_list_alloc();

    view_dispatcher_add_view(
        app->view_dispatcher, PassportSettingsAppViewMenu, submenu_get_view(app->submenu));
    view_dispatcher_add_view(
        app->view_dispatcher,
        PassportSettingsAppViewVarItemList,
        variable_item_list_get_view(app->variable_item_list));
    return app;
}

void passport_settings_app_free(PassportSettingsApp* app) {
    furi_assert(app);
    // Variable item list
    view_dispatcher_remove_view(app->view_dispatcher, PassportSettingsAppViewMenu);
    view_dispatcher_remove_view(app->view_dispatcher, PassportSettingsAppViewVarItemList);
    variable_item_list_free(app->variable_item_list);
    submenu_free(app->submenu);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    // Records
    furi_record_close(RECORD_GUI);
    free(app);
}

extern int32_t passport_settings_app(void* p) {
    UNUSED(p);
    PassportSettingsApp* app = passport_settings_app_alloc();

    // Load Passport Settings
    if(!(passport_settings_load(&app->settings))) {
        app->settings.background = true;
        app->settings.image = true;
        app->settings.name = true;
        app->settings.mood = true;
        app->settings.level = true;
        app->settings.xp_text = true;
        app->settings.xp_mode = 0;
        app->settings.multipage = true;
        passport_settings_save(&app->settings);
    }

    scene_manager_next_scene(app->scene_manager, PassportSettingsAppSceneStart);

    view_dispatcher_run(app->view_dispatcher);
    passport_settings_app_free(app);
    return 0;
}
