const char* ssid     = "KM";
const char* password = "Pavel9317";

const char* mqtt_broker = "10.0.0.7";
const int mqtt_port = 1883;
const char* mqtt_username = "homeassistant";
const char* mqtt_password = "quuSh5aem2aey1jeejoo9ul0She7IeshiepoYeing9aew0Tied2tohph3egheixu";

byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};

const char* movement_topic = "movement";
const char* state_topic = "state";

const int ledUp = 5;
const int ledDown = 4;

bool goUp = false;
bool goDown = false;
bool stop = false;

unsigned long start_time = 0;
unsigned long state_time = 0;
int current_position = 0;
int path_length = 10;
char* state_message;
char buffer [256];