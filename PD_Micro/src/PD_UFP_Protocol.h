
/**
 * PD_UFP_Protocol.c
 *
 *  Updated on: Jan 4, 2021
 *      Author: Ryan Ma
 *
 * Minimalist USB PD implement with only UFP(device) functionality
 * Requires PD PHY to do automatic GoodCRC response on valid SOP messages.
 * Requires only stdint.h, stdbool.h and string.h
 * No use of bit-field for better cross-platform compatibility
 *
 * Support PD3.0 PPS
 * Do not support extended message. Not necessary for PD trigger and PPS.
 * 
 * Reference: USB_PD_R2_0 V1.3 - 20170112
 *            USB_PD_R3_0 V2.0 20190829 + ECNs 2020-12-10
 *            - Chapter 6. Protocol Layer
 *
 */

#ifndef PD_UFP_PROTOCOL_H
#define PD_UFP_PROTOCOL_H

#include <stdbool.h>
#include <stdint.h>

/* For use in PD_protocol_get_power_info() */
#define PD_V(v)     ((uint16_t)(v * 20))
#define PD_A(a)     ((uint16_t)(a * 100))

/* For use in PD_protocol_set_PPS_option() */
#define PPS_V(v)    ((uint16_t)(v * 50))
#define PPS_A(a)    ((uint8_t)(a * 20))


#define PD_PROTOCOL_EVENT_SRC_CAP       (1 << 0)
#define PD_PROTOCOL_EVENT_PS_RDY        (1 << 1)
#define PD_PROTOCOL_EVENT_ACCEPT        (1 << 2)
#define PD_PROTOCOL_EVENT_REJECT        (1 << 3)

typedef uint8_t PD_protocol_event_t;

enum PD_power_option_t {
    PD_POWER_OPTION_MAX_5V      = 0,
    PD_POWER_OPTION_MAX_9V      = 1,
    PD_POWER_OPTION_MAX_12V     = 2,
    PD_POWER_OPTION_MAX_15V     = 3,
    PD_POWER_OPTION_MAX_20V     = 4,
    PD_POWER_OPTION_MAX_VOLTAGE = 5,
    PD_POWER_OPTION_MAX_CURRENT = 6,
    PD_POWER_OPTION_MAX_POWER   = 7,
};

enum PD_power_data_obj_type_t {   /* Power data object type */
    PD_PDO_TYPE_FIXED_SUPPLY    = 0,
    PD_PDO_TYPE_BATTERY         = 1,
    PD_PDO_TYPE_VARIABLE_SUPPLY = 2,
    PD_PDO_TYPE_AUGMENTED_PDO   = 3     /* USB PD 3.0 */
};

typedef struct {
    const char *name;
    uint8_t id;
    uint8_t spec_rev;
    uint8_t num_of_obj;
} PD_msg_info_t;

typedef struct {
    enum PD_power_data_obj_type_t type;
    uint16_t min_v;     /* Voltage in 50mV units */
    uint16_t max_v;     /* Voltage in 50mV units */
    uint16_t max_i;     /* Current in 10mA units */
    uint16_t max_p;     /* Power in 250mW units */
} PD_power_info_t;

struct PD_msg_state_t;
typedef struct {
    const struct PD_msg_state_t *msg_state;
    const char *tx_msg_name;
    uint16_t tx_msg_header;
    uint16_t rx_msg_header;
    uint8_t message_id;

    uint16_t PPS_voltage;
    uint8_t PPS_current;

    enum PD_power_option_t power_option;
    uint32_t power_data_obj[6];
    uint8_t power_data_obj_count;
    uint8_t power_data_obj_selected;
} PD_protocol_t;

void PD_protocol_handle_msg(PD_protocol_t *p, uint16_t header, uint32_t *obj, PD_protocol_event_t *events);
bool PD_protocol_respond(PD_protocol_t *p, uint16_t *h, uint32_t *obj);

void PD_protocol_create_get_src_cap(PD_protocol_t *p, uint16_t *header);
void PD_protocol_create_request(PD_protocol_t *p, uint16_t *header, uint32_t *obj);

static inline uint8_t PD_protocol_get_selected_power(PD_protocol_t *p) { return p->power_data_obj_selected; }
static inline uint16_t PD_protocol_get_PPS_voltage(PD_protocol_t *p) { return p->PPS_voltage; } /* Voltage in 20mV units */
static inline uint8_t  PD_protocol_get_PPS_current(PD_protocol_t *p) { return p->PPS_current; } /* Current in 50mA units */
        
bool PD_protocol_get_power_info(PD_protocol_t *p, uint8_t index, PD_power_info_t *power_info);
bool PD_protocol_get_tx_msg_info(PD_protocol_t *p, PD_msg_info_t *msg_info);
bool PD_protocol_get_rx_msg_info(PD_protocol_t *p, PD_msg_info_t *msg_info);

/* Set Fixed and Variable power option */
bool PD_protocol_set_power_option(PD_protocol_t *p, enum PD_power_option_t option);
bool PD_protocol_select_power(PD_protocol_t *p, uint8_t index);

/* Set PPS Voltage in 20mV units, Current in 50mA units. return true if re-send request is needed
   strict=true, If PPS setting is not qualified, return false, nothing is changed.
   strict=false, if PPS setting is not qualified, fall back to regular power option */
bool PD_protocol_set_PPS(PD_protocol_t * p, uint16_t PPS_voltage, uint8_t PPS_current, bool strict);  

void PD_protocol_reset(PD_protocol_t *p);
void PD_protocol_init(PD_protocol_t *p);

#endif
