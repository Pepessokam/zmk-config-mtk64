#include <device.h>
#include <drivers/uart.h>
#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>

const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_shell_uart));

static void send_layer_change(uint8_t layer) {
    if (!device_is_ready(uart_dev)) return;

    char msg[32];
    snprintf(msg, sizeof(msg), "LAYER:%d\n", layer);
    uart_tx(uart_dev, msg, strlen(msg), SYS_FOREVER_MS);
}

static int layer_listener(const zmk_event_t *eh) {
    const struct layer_state_changed *ev = as_layer_state_changed(eh);
    if (ev) {
        send_layer_change(ev->state);
    }
    return 0;
}

ZMK_LISTENER(layer_serial, layer_listener);
ZMK_SUBSCRIPTION(layer_serial, layer_state_changed);