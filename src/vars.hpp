const int mqtt_port = 1883;
const char* mqtt_username = "homeassistant";
const char* mqtt_password = "quuSh5aem2aey1jeejoo9ul0She7IeshiepoYeing9aew0Tied2tohph3egheixu";

byte mac[] = {0x00, 0x10, 0xFA, 0x6E, 0x38, 0x4A};

const char* movement_topic = "movement";
const char* state_topic = "state";
const char* tilt_topic = "tilt";

const int pinUp = 4;
const int pinDown = 5;

bool goUp = false;
bool goDown = false;
bool stop = false;

String tiltDirection = "stop";
String tiltFully = "no";
String stepTiltDirection = "none";
unsigned long tiltTime = 0;
unsigned long pauseTime = 0;
unsigned long fullTiltTime = 0;
unsigned long stepTiltTime = 0;
int tilt_position = 0;

unsigned long start_time = 0;
unsigned long state_time = 0;
int current_position = 100;
int path_length = 20;
int tilt_length = 1;
char* state_message;
char* tilt_message;
char buffer [256];
