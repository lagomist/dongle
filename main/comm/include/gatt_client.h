#pragma once

#include <cstdint>
#include <string_view>


namespace Wrapper {

namespace BLE {

namespace Client {

enum class EvtType : uint8_t {
    IDLE,
    SCANTING_EVT,
    SCAN_TIMEOUT_EVT,           // 扫描超时
    CONNECTING_EVT,
    CONNECT_TIMEOUT_EVT,        // 连接超时
    CONNECTED_EVT,              // 连接成功
    SERVICE_DISCOVER_EVT,       // 服务发现
    SEND_COMPLETE_EVT,          // 数据发送完成
    DISCONNECTED_EVT,           // 连接断开
};

constexpr std::string_view evt_to_str(EvtType evt) {
    switch (evt) {
    case EvtType::IDLE:                 return "idle";
    case EvtType::SCANTING_EVT:         return "scaning";
    case EvtType::SCAN_TIMEOUT_EVT:     return "scan timeout";
    case EvtType::CONNECTING_EVT:       return "connecting";
    case EvtType::CONNECT_TIMEOUT_EVT:  return "timeout";
    case EvtType::CONNECTED_EVT:        return "connected";
    case EvtType::SERVICE_DISCOVER_EVT: return "service discoverd";
    case EvtType::SEND_COMPLETE_EVT:    return "sended";
    case EvtType::DISCONNECTED_EVT:     return "disconnected";
    default:
        break;
    }
    return "unknown";
}

struct AdvReport {
    char name[32];
    uint8_t addr[6];
    int8_t tx_power;
    int8_t rssi;
};

struct CharHandle {
    uint16_t uuid;
    uint16_t char_handle;       /**< Handle of the characteristic as provided by a discovery. */
    uint16_t cccd_handle;       /**< Handle of the CCCD of the characteristic as provided by a discovery. */
};

// 特征值信息
struct CharacteristicProperty {
    uint16_t uuid;           // 特征UUID
    uint16_t handle;         // 特征声明句柄
    uint16_t value_handle;   // 特征值句柄
    uint8_t properties;      // 特征属性
    uint16_t cccd_handle;    // 描述符句柄
};

// 服务信息
struct ServiceProperty {
    uint16_t uuid;           // 服务UUID
    uint16_t start_handle;   // 服务起始句柄
    uint16_t end_handle;     // 服务结束句柄
    CharacteristicProperty characteristics[5]; // 特征数组
    uint16_t char_count;      // 特征数量
};

// 设备GATT数据库
struct GattDatabase {
    ServiceProperty services[5]; // 服务数组
    uint16_t service_count;      // 服务数量
    uint16_t conn_handle;        // 连接句柄
};

using EvtCallback = void (*)(EvtType evt, uint16_t handle);
using ScanCallback = void (*)(AdvReport report);
using RecvCallback = void (*)(uint16_t handle, const uint8_t *data, uint16_t len);
using DbDisCallback = void (*)(GattDatabase* database);

int init();

int discovery_service(uint16_t srv_uuid);
int discovery_device(std::string_view dev_name);
int primay_serivce_discover();

int send(uint16_t char_handle, const void * data, uint16_t length);
int send(const void * data, uint16_t length);

int register_conn_handle(uint16_t conn_handle);

int notif_config(uint16_t cccd_handle, bool notification_enable);
int notif_enable();
void scan_start(uint16_t timeout_sec = 0);
int connection(uint8_t addr[6], uint16_t timeout_sec);
int disconnection();

void register_evt_callback(EvtCallback cb);
void register_scan_callback(ScanCallback cb);
void register_recv_callback(RecvCallback cb);
void register_db_callback(DbDisCallback cb);

// std::vector<uint8_t> adv_data_prase(uint8_t type, const uint8_t adv_data[], uint8_t adv_len);
std::string_view get_scan_adv_name(const uint8_t adv_data[], uint8_t adv_len);

} /* Client */

} /* BLE */

} /* Wrapper */
