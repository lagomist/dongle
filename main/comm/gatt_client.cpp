#include "gatt_client.h"
#include "sdk_config.h"
#include "sdk_common.h"
#include "ble.h"
#include "ble_gatt.h"
#include "ble_gattc.h"
#include "ble_db_discovery.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh.h"
#include "ble_srv_common.h"
#include "nrf_ble_scan.h"
#include "nrf_ble_gatt.h"
#include "app_error.h"
#include <vector>
#include <cstdint>
#include <string_view>

#define NRF_LOG_MODULE_NAME gatt_client
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

namespace Wrapper {

namespace BLE {

namespace Client {

#define OPCODE_LENGTH 1
#define HANDLE_LENGTH 2
#ifndef NRF_BLE_GQ_QUEUE_SIZE
#define NRF_BLE_GQ_QUEUE_SIZE 4
#endif

// 应用程序BLE事件监视者优先级，应用程序不能修改该数值
#define APP_BLE_BAP_OBSERVER_PRIO       2
#define APP_BLE_GATT_OBSERVER_PRIO      3

// 定义GATT 128位UUID基数
constexpr static const ble_uuid128_t GATT_BASE_UUID = {0x40, 0xE3, 0x4A, 0x1D, 0xC2, 0x5F, 0xB0, 0x9C, 0xB7, 0x47, 0xE6, 0x43, 0x00, 0x00, 0x53, 0x86};
constexpr static const uint8_t APP_BLE_CONN_CFG_TAG = 1;
constexpr static const uint16_t APP_BLE_SCAN_INTERVAL   = 512;      // 扫描间隔(*0.625)320ms
constexpr static const uint16_t APP_BLE_SCAN_WINDOW     = 160;      // 扫描时间(*0.625)100ms



struct gattc_profile {
    uint8_t         uuid_type;        // UUID 类型
    uint16_t        conn_handle;      // 连接句柄
    uint16_t        srv_uuid;
    CharHandle      characteristic;
    ScanCallback    scan_handler;
    EvtCallback     evt_handler;
    RecvCallback    recv_handler;
    DbDisCallback   db_handler;
};
static gattc_profile _profile;
NRF_BLE_GATT_DEF(_gatt_inst);
NRF_BLE_GQ_DEF(_ble_gatt_queue, NRF_SDH_BLE_CENTRAL_LINK_COUNT, NRF_BLE_GQ_QUEUE_SIZE);
NRF_BLE_SCAN_DEF(_scan_inst);
BLE_DB_DISCOVERY_DEF(_db_disc);

static uint16_t _gattc_max_data_len = (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - OPCODE_LENGTH - HANDLE_LENGTH);

static void gatt_error_handler(uint32_t nrf_error, void * p_ctx, uint16_t conn_handle) {
    NRF_LOG_ERROR("A GATT Client error has occurred on conn_handle: 0X%X", conn_handle);
    APP_ERROR_HANDLER(nrf_error);
}

// DB发现事件处理函数
static void db_disc_handler(ble_db_discovery_evt_t * p_evt) {
    if (p_evt->evt_type != BLE_DB_DISCOVERY_COMPLETE) return;

    ble_gatt_db_char_t * p_chars = p_evt->params.discovered_db.charateristics;
    if (_profile.srv_uuid == BLE_UUID_UNKNOWN) {
        // 未设置服务UUID，回调处理
        if (_profile.db_handler != nullptr) {
            uint16_t service_uuid = p_evt->params.discovered_db.srv_uuid.uuid;
            uint16_t char_count = p_evt->params.discovered_db.char_count;
            CharHandle char_array[12];
            for (int i = 0; i < char_count; i++) {
                char_array[i].uuid = p_chars[i].characteristic.uuid.uuid;
                char_array[i].char_handle = p_chars[i].characteristic.handle_value;
                char_array[i].cccd_handle = p_chars[i].cccd_handle;
            }
            _profile.db_handler(service_uuid, char_array, char_count);
        }
        return;
    }
    // found service uuid and characteristic uuid
    if (_profile.srv_uuid == p_evt->params.discovered_db.srv_uuid.uuid) {
        for (int i = 0; i < p_evt->params.discovered_db.char_count; i++) {
            if (_profile.characteristic.uuid == p_chars[i].characteristic.uuid.uuid) {
                _profile.characteristic.char_handle = p_chars[i].characteristic.handle_value;
                _profile.characteristic.cccd_handle = p_chars[i].cccd_handle;
            }
        }
        if (_profile.evt_handler != nullptr) {
            _profile.evt_handler(SERVICE_DISCOVER_EVT, p_evt->conn_handle);
        }
        register_conn_handle(p_evt->conn_handle);
    }
}

static void ble_gap_evt_handler(ble_evt_t const * p_ble_evt, void * p_context) {
    ret_code_t            err_code;
    ble_gap_evt_t const * p_gap_evt = &p_ble_evt->evt.gap_evt;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Ble connected");
            _profile.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            register_conn_handle(p_ble_evt->evt.gap_evt.conn_handle);

            // 启动服务发现，GATT客户端会等待发现完成事件
            err_code = ble_db_discovery_start(&_db_disc, p_ble_evt->evt.gap_evt.conn_handle);
            APP_ERROR_CHECK(err_code);

            // Resume scanning.
            // scan_start();
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Ble disconnected. reason: 0x%x", p_gap_evt->params.disconnected.reason);
            _profile.conn_handle = BLE_CONN_HANDLE_INVALID;
            if (_profile.evt_handler != nullptr)
                _profile.evt_handler(DISCONNECTED_EVT, p_ble_evt->evt.gap_evt.conn_handle);
            // Resume scanning.
            scan_start();
            break;

        case BLE_GAP_EVT_TIMEOUT:
            NRF_LOG_WARNING("Connection Request timed out.");
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // 不支持配对
            NRF_LOG_DEBUG("sec params request.\n");
            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;
				 
        case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
            NRF_LOG_DEBUG("connect param update request.\n");
            // 接受对端设备的连接参数更新请求
            err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                                                    &p_gap_evt->params.conn_param_update_request.conn_params);
            APP_ERROR_CHECK(err_code);
            break;
				 
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
            NRF_LOG_DEBUG("PHY update request.\n");
            ble_gap_phys_t const phys = {
                .tx_phys = BLE_GAP_PHY_AUTO,
                .rx_phys = BLE_GAP_PHY_AUTO,
            };
            // 响应从机PHY请求
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
            break;
        }
        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.\n");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;
        default:
            break;
    }
}

static void ble_observer_evt_handler(ble_evt_t const * p_ble_evt, void * p_context) {
    if (p_ble_evt == nullptr || _profile.evt_handler == nullptr) return;
    // 检查连接句柄是否有效
    if ( (_profile.conn_handle == BLE_CONN_HANDLE_INVALID) || (_profile.conn_handle != p_ble_evt->evt.gap_evt.conn_handle)) {
        return;
    }
    // 判断事件类型
    switch (p_ble_evt->header.evt_id) {
        case BLE_GATTC_EVT_HVX: {
            // 通知或指示事件
            NRF_LOG_DEBUG("BLE_GATTC_EVT_HVX");
            if (_profile.recv_handler == nullptr) return;
            uint16_t handle = p_ble_evt->evt.gattc_evt.params.hvx.handle;
            const uint8_t *p_data = p_ble_evt->evt.gattc_evt.params.hvx.data;
            uint16_t data_len = p_ble_evt->evt.gattc_evt.params.hvx.len;
            _profile.recv_handler(handle, p_data, data_len);
            break;
        }
        case BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE: {
            // Write without Response transmission complete.
            NRF_LOG_DEBUG("BLE_GATTC_EVT_WRITE_CMD_TX_COMPLETE");
            _profile.evt_handler(SEND_COMPLETE_EVT, p_ble_evt->evt.gap_evt.conn_handle);
            break;
        }
        default: break;
    }
}

static void ble_scan_evt_handler(scan_evt_t const * p_scan_evt) {
    ret_code_t err_code;
	
    switch(p_scan_evt->scan_evt_id) {
        case NRF_BLE_SCAN_EVT_CONNECTING_ERROR: {
            NRF_LOG_DEBUG("NRF_BLE_SCAN_EVT_CONNECTING_ERROR");
            err_code = p_scan_evt->params.connecting_err.err_code;
            APP_ERROR_CHECK(err_code);
            break;
        }
			
        case NRF_BLE_SCAN_EVT_FILTER_MATCH: {
            NRF_LOG_DEBUG("device name filter match");
            ble_gap_evt_adv_report_t const * p_adv = p_scan_evt->params.filter_match.p_adv_report;
            if (_profile.scan_handler) {
                _profile.scan_handler(p_adv->data.p_data, p_adv->data.len);
            }
            break;
        }

        case NRF_BLE_SCAN_EVT_NOT_FOUND: {
            ble_gap_evt_adv_report_t const * p_adv = p_scan_evt->params.p_not_found;
            if (_profile.scan_handler) {
                _profile.scan_handler(p_adv->data.p_data, p_adv->data.len);
            }
            break;
        }

        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT: {
            NRF_LOG_WARNING("Scan timed out.");
            if (_profile.evt_handler) {
                _profile.evt_handler(SCAN_TIMEOUT_EVT, 0);
            }
            break;
        }
        default: break;
    }
}

static void gatt_evt_handler(nrf_ble_gatt_t * p_gatt, nrf_ble_gatt_evt_t const * p_evt) {
    // 如果是MTU交换事件
    if (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED) {
        // 设置GATT通信服务的有效数据长度（MTU-opcode-handle=MTU大小-1-2）
        _gattc_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        NRF_LOG_INFO("Ble transmit max data length set to %d byte", _gattc_max_data_len);
    }
}

int notif_config(uint16_t cccd_handle, bool notification_enable) {
    // 若连接句柄无效或CCCD句柄无效，返回错误代码：当前状态无效，不允许执行操作
    if ( (_profile.conn_handle == BLE_CONN_HANDLE_INVALID) ||(cccd_handle == BLE_GATT_HANDLE_INVALID)) {
        return NRF_ERROR_INVALID_STATE;
    }
    nrf_ble_gq_req_t cccd_req;
    // 取得写入的数据（使能/关闭Notify）
    uint8_t  cccd[BLE_CCCD_VALUE_LEN];
    uint16_t cccd_val = notification_enable ? BLE_GATT_HVX_NOTIFICATION : 0;

    memset(&cccd_req, 0, sizeof(nrf_ble_gq_req_t));
    cccd[0] = LSB_16(cccd_val);
    cccd[1] = MSB_16(cccd_val);
    // 初始化写入参数
    cccd_req.type                        = NRF_BLE_GQ_REQ_GATTC_WRITE;
    cccd_req.error_handler.cb            = gatt_error_handler;
    cccd_req.error_handler.p_ctx         = nullptr;
    cccd_req.params.gattc_write.handle   = cccd_handle;
    cccd_req.params.gattc_write.len      = BLE_CCCD_VALUE_LEN;
    cccd_req.params.gattc_write.offset   = 0;
    cccd_req.params.gattc_write.p_value  = cccd;
    cccd_req.params.gattc_write.write_op = BLE_GATT_OP_WRITE_REQ;
    cccd_req.params.gattc_write.flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE;
    // 执行写操作
    return nrf_ble_gq_item_add(&_ble_gatt_queue, &cccd_req, _profile.conn_handle);
}

int notif_enable() {
    return notif_config(_profile.characteristic.cccd_handle, true);
}

void scan_start(uint32_t timeout_ms) {
    ble_gap_scan_params_t params = {
        .active        = 0x01,
        .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
        .scan_phys     = BLE_GAP_PHY_1MBPS,
        .interval      = APP_BLE_SCAN_INTERVAL,
        .window        = APP_BLE_SCAN_WINDOW,
        .timeout       = (uint16_t )(timeout_ms / 10),
    };
    nrf_ble_scan_params_set(&_scan_inst, &params);
    nrf_ble_scan_start(&_scan_inst);
}

std::vector<uint8_t> adv_data_prase(uint8_t type, const uint8_t adv_data[], uint8_t adv_len) {
    uint8_t index = 0;
    while (index < adv_len) {
        uint8_t field_length = adv_data[index];
        uint8_t field_type = adv_data[index + 1];

        if(field_type == type) {
            const uint8_t *buf = &adv_data[index + 2];
            uint8_t len = field_length - 1; 
					
            return std::vector<uint8_t>(buf, buf + len);
        }
        index += field_length + 1;                    
    }
    
    return {};
}

std::string_view get_scan_adv_name(const uint8_t adv_data[], uint8_t adv_len) {
    std::vector<uint8_t> res = adv_data_prase(BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, adv_data, adv_len);
    if (res.empty()) {
        res = adv_data_prase(BLE_GAP_AD_TYPE_SHORT_LOCAL_NAME, adv_data, adv_len);
        if (res.empty())
            return {};
    }
    res.push_back('\0');
    return {(char *)res.data(), res.size()};
}

int send(uint16_t char_handle, void * data, uint16_t length) {
    nrf_ble_gq_req_t write_req;
    memset(&write_req, 0, sizeof(nrf_ble_gq_req_t));
    // 检查数据长度是否正确
    if (length > _gattc_max_data_len) {
        NRF_LOG_WARNING("Content too long.");
        return NRF_ERROR_INVALID_PARAM;
    }
    // 检查连接句柄是否有效
    if (_profile.conn_handle == BLE_CONN_HANDLE_INVALID) {
        NRF_LOG_WARNING("Connection handle invalid.");
        return NRF_ERROR_INVALID_STATE;
    }
    // 初始化写入参数
    write_req.type                        = NRF_BLE_GQ_REQ_GATTC_WRITE;
    write_req.error_handler.cb            = gatt_error_handler;
    write_req.error_handler.p_ctx         = nullptr;
    write_req.params.gattc_write.handle   = char_handle;
    write_req.params.gattc_write.len      = length;
    write_req.params.gattc_write.offset   = 0;
    write_req.params.gattc_write.p_value  = (uint8_t *)data;
    write_req.params.gattc_write.write_op = BLE_GATT_OP_WRITE_CMD;
    write_req.params.gattc_write.flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE;
    // 执行写操作
    return nrf_ble_gq_item_add(&_ble_gatt_queue, &write_req, _profile.conn_handle);
}

int send(void * data, uint16_t length) {
    return send(_profile.characteristic.char_handle, data, length);
}

int register_conn_handle(uint16_t conn_handle) {
    _profile.conn_handle = conn_handle;
    return nrf_ble_gq_conn_handle_register(&_ble_gatt_queue, conn_handle);
}


int discovery_service(uint16_t srv_uuid) {
    ble_uuid_t    app_uuid;
    app_uuid.type = _profile.uuid_type;
    app_uuid.uuid = srv_uuid;
    return ble_db_discovery_evt_register(&app_uuid);
}

int discovery_device(std::string_view dev_name) {
    // 向扫描器添加设备名称过滤器
    ret_code_t err_code = nrf_ble_scan_filter_set(&_scan_inst, SCAN_NAME_FILTER, dev_name.data());
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_scan_filters_enable(&_scan_inst, NRF_BLE_SCAN_NAME_FILTER, false);
    return err_code;
}

void register_evt_callback(EvtCallback cb) {
    _profile.evt_handler = cb;
}

void register_scan_callback(ScanCallback cb) {
    _profile.scan_handler = cb;
}

void register_recv_callback(RecvCallback cb) {
    _profile.recv_handler = cb;
}

void register_db_callback(DbDisCallback cb) {
    _profile.db_handler = cb;
}

static void ble_scan_init(void) {
    ret_code_t          err_code;
    ble_gap_scan_params_t scan_param = {
        .active        = 0x01,
        .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,
        .scan_phys     = BLE_GAP_PHY_1MBPS,
        .interval      = APP_BLE_SCAN_INTERVAL,
        .window        = APP_BLE_SCAN_WINDOW,
        .timeout       = 0,
    };
    nrf_ble_scan_init_t init_scan;
	
    memset(&init_scan, 0, sizeof(init_scan));
    // 自动连接设置为false
    init_scan.connect_if_match = true;
    // 使用初始化结构体中的扫描参数配置扫描器，这里p_scan_param指向定义的扫描参数
    init_scan.p_scan_param     = &scan_param;
    // conn_cfg_tag设置为1
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;
    // 初始化扫描器
    err_code = nrf_ble_scan_init(&_scan_inst, &init_scan, ble_scan_evt_handler);
    APP_ERROR_CHECK(err_code);
}


static int ble_stack_init(void) {
    ret_code_t err_code;
    // 请求使能SoftDevice，该函数中会根据sdk_config.h文件中低频时钟的设置来配置低频时钟
    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);
    
    // 定义保存应用程序RAM起始地址的变量
    uint32_t ram_start = 0;
    // 使用sdk_config.h文件的默认参数配置协议栈，获取应用程序RAM起始地址，保存到变量ram_start
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // 使能BLE协议栈
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);
    return err_code;
}

int init() {
    ret_code_t err_code;
    // 初始化蓝牙协议栈
    ble_stack_init();
    err_code = nrf_ble_gatt_init(&_gatt_inst, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_ble_gatt_att_mtu_central_set(&_gatt_inst, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
    // 注册BLE事件回调函数
    NRF_SDH_BLE_OBSERVER(m_gap_obs, APP_BLE_BAP_OBSERVER_PRIO, ble_gap_evt_handler, NULL);
    NRF_SDH_BLE_OBSERVER(m_gatt_obs, APP_BLE_GATT_OBSERVER_PRIO, ble_observer_evt_handler, &_profile);
    _profile.conn_handle = BLE_CONN_HANDLE_INVALID;
    // 将自定义的UUID基数写入到协议栈
    err_code = sd_ble_uuid_vs_add(&GATT_BASE_UUID, &_profile.uuid_type);
    APP_ERROR_CHECK(err_code);

    ble_db_discovery_init_t db_init = {
        .evt_handler = db_disc_handler,
        .p_gatt_queue = &_ble_gatt_queue,
    };
    // 初始化DB发现模块
    err_code = ble_db_discovery_init(&db_init);
    APP_ERROR_CHECK(err_code);
    ble_scan_init();
    NRF_LOG_INFO("init success.");
    return err_code;
}

} /* Client */

} /* BLE */

} /* Wrapper */
