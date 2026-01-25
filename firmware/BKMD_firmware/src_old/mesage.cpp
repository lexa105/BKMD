enum class LogLevel : uint8_t { INFO, WARN, ERROR, DEBUG };

struct LogMsg {
  uint32_t t_ms;
  LogLevel level;
  uint16_t code;        // optional numeric code
  char tag[12];         // module tag: "BLE", "DEC", "GPIO"...
  char text[64];        // message text (truncated)
};


static constexpr uint32_t LOG_QUEUE_LEN = 32;
static QueueHandle_t logQ = nullptr;

static volatile uint32_t gLogDrop = 0;
