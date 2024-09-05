// NVS
extern void utils_nvs_init(void);
extern esp_err_t utils_nvs_get_str(const char *  , char * , size_t );
extern void utils_nvs_erase_all(void);
extern esp_err_t utils_nvs_get_int(const char * , int16_t *);
extern void utils_nvs_erase_key(const char *);
extern void utils_nvs_set_int(const char *  , int16_t);
extern void load_settings_nvs(void);
extern void utils_nvs_set_str(const char * , const char *);
//Analyse Packet UART
extern void process_uart_packet(const char *);

//uart
extern void uart_write_string(const char *);
extern void uart_write_string_ln(const char *);
extern void uart_write_number(uint8_t);



// tcp
extern void tcp_ip_client_send_str(const char *);
extern void tcp_ip_client_send_str(const char *);
extern void tcpip_client_task(void);
extern void sendHBT (void);
extern void tcp_ip_client_send_str(const char *);


// wifi
extern bool connect_to_wifi(char *, char *);
extern bool connect_to_wifi(char *, char *);
extern void event_handler(void* , esp_event_base_t , int32_t , void* );
extern void smartconfig_example_task(void * );
extern void wifi_init_sta(void);
extern esp_err_t _http_event_handler(esp_http_client_event_t *);
// fota
extern void http_fota (void);


// general
extern void resolve_hostname(const char *);
extern void RestartDevice (void);

// hardware
extern void Out4094Byte (unsigned char);
extern void gpio_read_n_act(void);
extern void ICH_init();
extern void Out4094 (unsigned char);
extern void BlinkLED (void);
extern void GeneratePulsesInBackGround (void);
extern void TestCoin (void);
extern void Test4094 (void);
extern void s2p_init(void);
extern void console_uart_init(void);
extern void read_mac_address(void);
extern void led_set_level(gpio_num_t , int);
extern void status_leds_init();
extern void leds_update_task();
extern void set_led_state(Led_State_t);
extern void TestRGB (void);





// timer
extern uint32_t millis(void);

// misc
extern bool extractSubstring(const char* , char* );
extern void resolve_hostname(const char *);
extern uint32_t millis(void);

// void
extern void InitMqtt (void);
extern void mqtt_app_start(void);
// extern void publish_message(const char *, esp_mqtt_client_handle_t *)


//BT
extern void bt_init(void);