#include <zephyr/logging/log.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>

LOG_MODULE_REGISTER(layer_notify, LOG_LEVEL_INF);

int layer_state_changed_listener(const struct zmk_event_header *eh) {
    const struct layer_state_changed *ev = as_layer_state_changed(eh);

    // レイヤー番号と状態を UART に出力
    //printk("Layer %d %s\n", ev->layer, ev->state ? "on" : "off");

    //return 0;

    // printkはタイミングによってはフリーズの原因に
    // 代わりに軽量なログで出力確認（ログレベルはLOG_LEVEL_INF以上にする）
    LOG_INF("Layer %d %s", ev->layer, ev->state ? "on" : "off");

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(layer_notify, layer_state_changed_listener);
ZMK_SUBSCRIPTION(layer_notify, layer_state_changed);