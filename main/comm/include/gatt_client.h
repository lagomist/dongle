#pragma once

#include <stdint.h>
#include <string_view>


namespace Wrapper {

namespace BLE {

namespace Client {

enum EvtType : uint8_t {
    SCAN_TIMEOUT_EVT,           // 扫描超时
    SERVICE_DISCOVER_EVT,       // 服务发现
    SEND_COMPLETE_EVT,          // 数据发送完成
    DISCONNECTED_EVT,           // 连接断开
};

struct CharHandle {
    uint16_t uuid;
    uint16_t char_handle;       /**< Handle of the characteristic as provided by a discovery. */
    uint16_t cccd_handle;       /**< Handle of the CCCD of the characteristic as provided by a discovery. */
};

using EvtCallback = void (*)(EvtType evt, uint16_t handle);
using ScanCallback = void (*)(const uint8_t *adv_data, uint8_t adv_len);
using RecvCallback = void (*)(uint16_t handle, const uint8_t *data, uint16_t len);
using DbDisCallback = void (*)(uint16_t srv_uuid, CharHandle char_uuid[], uint16_t count);

int init();

int discovery_service(uint16_t srv_uuid);
int discovery_device(std::string_view dev_name);

int send(uint16_t char_handle, void * data, uint16_t length);
int send(void * data, uint16_t length);

int register_conn_handle(uint16_t conn_handle);

int notif_config(uint16_t cccd_handle, bool notification_enable);
int notif_enable();
void scan_start(uint16_t timeout_sec = 0);

void register_evt_callback(EvtCallback cb);
void register_scan_callback(ScanCallback cb);
void register_recv_callback(RecvCallback cb);
void register_db_callback(DbDisCallback cb);

// std::vector<uint8_t> adv_data_prase(uint8_t type, const uint8_t adv_data[], uint8_t adv_len);
std::string_view get_scan_adv_name(const uint8_t adv_data[], uint8_t adv_len);

} /* Client */

} /* BLE */

} /* Wrapper */
