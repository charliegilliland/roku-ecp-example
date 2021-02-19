#ifndef __ROKU_H__
#define __ROKU_H__

/* Roku ECP Keypress Values */
#define ROKU_VOL_UP "VolumeUp"
#define ROKU_VOL_DOWN "VolumeDown"
#define ROKU_VOL_MUTE "VolumeMute"
#define ROKU_UP "Up"
#define ROKU_DOWN "Down"
#define ROKU_LEFT "Left"
#define ROKU_RIGHT "Right"
#define ROKU_SELECT "Select"
#define ROKU_BACK "Back"
#define ROKU_HOME "Home"
#define ROKU_POWER "Power"
#define ROKU_ENTER "Enter"
#define ROKU_SEARCH "Search"
#define ROKU_BACKSPACE "Backspace"
#define ROKU_INFO "Info"
#define ROKU_INSTANT_REPLAY "InstantReplay"
#define ROKU_PLAY "Play"
#define ROKU_FWD "Fwd"
#define ROKU_REV "Rev"
#define ROKU_INPUT_TUNER "InputTuner"
#define ROKU_INPUT_HDMI1 "InputHDMI1"
#define ROKU_INPUT_HDMI2 "InputHDMI2"
#define ROKU_INPUT_HDMI3 "InputHDMI3"
#define ROKU_INPUT_HDMI4 "InputHDMI4"
#define ROKU_INPUT_AV1 "InputAV1"


/* Keypress types */
#define ROKU_KEY_PRESS "/keypress/"
#define ROKU_KEY_DOWN "/keydown/"
#define ROKU_KEY_UP "/keyup/"


/* Search Parameters */
#define ROKU_SEARCH_PAYLOAD "M-SEARCH * HTTP/1.1\r\nHost: 239.255.255.250:1900\r\nMan: \"ssdp:discover\"\r\nST: roku:ecp\r\n"
#define ROKU_SEARCH_IP "239.255.255.250"
#define ROKU_SEARCH_PORT 1900

/* Contains IP address and device name for a roku_device */
struct roku_device {
    char ip_addr[15];
    char name[64];
};

/*
This function scans the network for Roku devices.

Parameters:
struct roku_device *devices // array to contain the results
uint8_t max_devices // the maximum number of devices to discover

Returns:
int num_devices // the number of devices found
*/
extern int roku_discover_devices(struct roku_device *devices, uint8_t max_devices);

/*
This function sends a keypress to the specified Roku device.

Parameters:
struct roku_device device // the device to send the command to
char *command // the command to send to the roku device
const char *keypress_type // the type of keypress (KeyDown, KeyUp, or Keypress)

Returns:
int status_code // the status code of the http request
*/
extern int roku_keypress(struct roku_device device, char *command, const char *keypress_type);

#endif /* __ROKU_H__ */