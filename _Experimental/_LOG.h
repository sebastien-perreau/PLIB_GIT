#ifndef __DEF_LOG
#define __DEF_LOG

typedef enum
{
    BASE_2  = 2,
    BASE_8  = 8,
    BASE_10 = 10,
    BASE_16 = 16
} LOG_BASE_t;

typedef enum
{
    LEVEL_0     = 0,        // LOG(...)         -> 0:00:00:00:000:000: your_str
    LEVEL_1     = 1,        // LOG_SHORT(...)   -> 0:000: your_str
    LEVEL_2     = 2,        // LOG_BLANCK(...)  -> your_str
} LOG_LEVEL_t;

#define p_string(s)                             (uint32_t) (s)
#define p_float(f)                              (uint32_t) (&f)

#define LOG_INTERNAL_X(level, str, N, ...)      log_wait_end_of_transmission(); log_frontend(str, level, ((uint32_t[]){ __VA_ARGS__ }), N)
#define LOG(str, ...)                           LOG_INTERNAL_X(LEVEL_0, str, COUNT_ARGUMENTS( __VA_ARGS__ ), __VA_ARGS__)
#define LOG_SHORT(str, ...)                     LOG_INTERNAL_X(LEVEL_1, str, COUNT_ARGUMENTS( __VA_ARGS__ ), __VA_ARGS__)
#define LOG_BLANCK(str, ...)                    LOG_INTERNAL_X(LEVEL_2, str, COUNT_ARGUMENTS( __VA_ARGS__ ), __VA_ARGS__)

void log_init(UART_MODULE id_uart, uint32_t data_rate);
void log_wait_end_of_transmission();
void log_frontend(const char *p_message, LOG_LEVEL_t level, const uint32_t *p_args, uint8_t nargs);

#endif
