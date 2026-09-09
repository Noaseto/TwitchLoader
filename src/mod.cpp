#include "mods/service.hpp"
#include "mods/svc/log.h"
#include "mods/svc/ui.h"

#include "config_var.hpp"
#include "i18n.hpp"
#include "panel_management.hpp"
#include "twitch/ws_client2.hpp"
#include "twitch/ws_constant.hpp"
#include "twitch_loader_service.h"

DEFINE_MOD();
IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(UiService, svc_ui);
IMPORT_SERVICE(ConfigService, svc_config);
IMPORT_SERVICE(HttpService, svc_http);
IMPORT_SERVICE(WebSocketService, svc_websocket);

extern "C" {

MOD_EXPORT ModResult mod_initialize(ModError* error) {
    // set config var
    ModResult result = register_variables(error);
    if (result != MOD_OK) {
        return result;
    }

    // init mod panel menu
    UiModsPanelDesc ui_mods_panel_desc = UI_MODS_PANEL_DESC_INIT;
    ui_mods_panel_desc.build = build_main_panel;
    result = svc_ui->register_mods_panel(mod_ctx, &ui_mods_panel_desc);
    if (result != MOD_OK) {
        return result;
    }

    // at startup, thread is not running, the toggle will launch the start
    if (get_bool_option(g_config_var_auto_start, false)) {
        result = g_ws2.toggle_socket();
        if (result != MOD_OK) {
            svc_log->error(mod_ctx, "Failed to establish a connection");
            return result;
        }
    }

    svc_log->info(mod_ctx, LOG_MOD_INIT.data());
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    if (g_ws2.is_started()) {
        if (!g_ws2.is_subscribed()) {
            if (g_ws2.get_user_id().empty()) {
                g_ws2.request_user_id();
            } else {
                TwitchSubscription topic = {.event_type = TwitchEventType::ChatMessage,
                    .type = SUBSCRIPTION_CHAT_MESSAGE.data(),
                    .version = SUBSCRIPTION_CHAT_MESSAGE_VERSION.data()};
                g_ws2.register_topic(topic);
            }
        }
    }
    // Then, we always do websocket polling
    g_ws2.poll();

    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    ModResult result = MOD_OK;
    if (g_ws2.is_started()) {
        result = g_ws2.toggle_socket();
    }
    svc_log->info(mod_ctx, LOG_MOD_STOP.data());
    return result;
}
}
