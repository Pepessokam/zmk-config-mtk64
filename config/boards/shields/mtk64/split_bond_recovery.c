/*
 * Split bond auto-recovery (central only).
 *
 * If the peripheral (left half) has lost its bond while the central still
 * holds the old LTK, every reconnect fails with PIN_OR_KEY_MISSING and the
 * halves never talk again until settings_reset is flashed to both sides.
 *
 * On that specific error for a link where we are the BLE central (i.e. a
 * split peripheral link, never the host link), drop the stale bond for that
 * address. ZMK's central rescans, re-reserves the same slot (address still
 * matches) and pairs from scratch.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) && IS_ENABLED(CONFIG_ZMK_SPLIT_BLE)

static bt_addr_le_t stale_addr;

static void unpair_work_cb(struct k_work *work) {
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(&stale_addr, addr, sizeof(addr));

    int err = bt_unpair(BT_ID_DEFAULT, &stale_addr);
    if (err) {
        LOG_ERR("Split bond recovery: unpair %s failed (%d)", addr, err);
    } else {
        LOG_WRN("Split bond recovery: removed stale bond for %s, re-pairing", addr);
    }
}

static K_WORK_DEFINE(unpair_work, unpair_work_cb);

static void recovery_security_changed(struct bt_conn *conn, bt_security_t level,
                                      enum bt_security_err err) {
    if (err != BT_SECURITY_ERR_PIN_OR_KEY_MISSING) {
        return;
    }

    struct bt_conn_info info;
    if (bt_conn_get_info(conn, &info) < 0 || info.role != BT_CONN_ROLE_CENTRAL) {
        return;
    }

    bt_addr_le_copy(&stale_addr, bt_conn_get_dst(conn));
    k_work_submit(&unpair_work);
}

BT_CONN_CB_DEFINE(split_bond_recovery_cb) = {
    .security_changed = recovery_security_changed,
};

#endif
