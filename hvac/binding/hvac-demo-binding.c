/*
 * Copyright (C) 2015, 2016 "IoT.bzh"
 * Copyright (C) 2016 Konsulko Group
 * Author "Romain Forlot"
 * Author "Jose Bolo"
 * Author "Scott Murray <scott.murray@konsulko.com>"
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <math.h>
#include <json-c/json.h>

#define AFB_BINDING_VERSION 2
#include <afb/afb-binding.h>

#define RED "/sys/class/leds/blinkm-3-9-red/brightness"
#define GREEN "/sys/class/leds/blinkm-3-9-green/brightness"
#define BLUE "/sys/class/leds/blinkm-3-9-blue/brightness"

#define CAN_DEV "vcan0"

#ifndef NULL
#define NULL 0
#endif

static const struct afb_binding_interface *interface;
static struct afb_event event;

/*****************************************************************************************/
/*****************************************************************************************/
/**											**/
/**											**/
/**	   SECTION: UTILITY FUNCTIONS							**/
/**											**/
/**											**/
/*****************************************************************************************/
/*****************************************************************************************/

/*
 * @brief Retry a function 3 times
 *
 * @param int function(): function that return an int wihtout any parameter
 *
 * @ return : 0 if ok, -1 if failed
 *
 */
static int retry( int(*func)())
{
	int i;

	for (i=0;i<4;i++)
	{
		if ( (*func)() >= 0)
		{
			return 0;
		}
		usleep(100000);
	}
	return -1;
}

/*****************************************************************************************/
/*****************************************************************************************/
/**											**/
/**											**/
/**	   SECTION: HANDLE CAN DEVICE							**/
/**											**/
/**											**/
/*****************************************************************************************/
/*****************************************************************************************/

// Initialize CAN hvac array that will be sent trough the socket
static struct {
	const char *name;
	uint8_t value;
} hvac_values[] = {
	{ "LeftTemperature", 21 },
	{ "RightTemperature", 21 },
	{ "Temperature", 21 },
	{ "FanSpeed", 0 },
	{ "ACEnabled", 0 },
	{ "LeftLed", 15 },
	{ "RightLed", 15 }
};

// Holds RGB combinations for each temperature
static struct {
	const int temperature;
	const int rgb[3];
} degree_colours[] = {
	{15, {0, 0, 229} },
	{16, {22, 0, 204} },
	{17, {34, 0, 189} },
	{18, {46, 0, 175} },
	{19, {58, 0, 186} },
	{20, {70, 0, 146} },
	{21, {82, 0, 131} },
	{22, {104, 0, 116} },
	{23, {116, 0, 102} },
	{24, {128, 0, 87} },
	{25, {140, 0, 73} },
	{26, {152, 0, 58} },
	{27, {164, 0, 43} },
	{28, {176, 0, 29} },
	{29, {188, 0, 14} },
	{30, {201, 0, 5} }
};

struct can_handler {
	int socket;
	bool simulation;
	char *send_msg;
	struct sockaddr_can txAddress;
};

struct led_paths {
	char *red;
	char *green;
	char *blue;
}led_paths_values = {
	.red = RED,
	.green = GREEN,
	.blue = BLUE
};

static struct can_handler can_handler = { .socket = -1, .simulation = false, .send_msg = "SENDING CAN FRAME"};

static int open_can_dev_helper()
{
	struct ifreq ifr;

	AFB_DEBUG("CAN Handler socket : %d", can_handler.socket);
	close(can_handler.socket);

	can_handler.socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (can_handler.socket < 0)
	{
		AFB_ERROR("socket could not be created");
	}
	else
	{
		// Attempts to open a socket to CAN bus
		strcpy(ifr.ifr_name, CAN_DEV);
		if(ioctl(can_handler.socket, SIOCGIFINDEX, &ifr) < 0)
		{
			AFB_ERROR("ioctl failed");
		}
		else
		{
			can_handler.txAddress.can_family = AF_CAN;
			can_handler.txAddress.can_ifindex = ifr.ifr_ifindex;

			// And bind it to txAddress
			if (bind(can_handler.socket, (struct sockaddr *)&can_handler.txAddress, sizeof(can_handler.txAddress)) < 0)
			{
				AFB_ERROR("bind failed");
			}
			else {
				return 0;
			}
		}
		close(can_handler.socket);
		can_handler.socket = -1;
	}
	return -1;
}

static int open_can_dev()
{
	int rc = retry(open_can_dev_helper);
	if(rc < 0)
	{
		AFB_ERROR("Open of interface %s failed. Falling back to simulation mode", CAN_DEV);
		can_handler.socket = 0;
		can_handler.simulation = true;
		can_handler.send_msg = "FAKE CAN FRAME";
		rc = 0;
	}
	return rc;
}

// Get original get temperature function from cpp hvacplugin code
static uint8_t to_can_temp(uint8_t value)
{
	int result = ((0xF0 - 0x10) / 15) * (value - 15) + 0x10;
	if (result < 0x10)
		result = 0x10;
	if (result > 0xF0)
		result = 0xF0;

	return (uint8_t)result;
}

static uint8_t read_temp_left_zone()
{
	return hvac_values[0].value;
}

static uint8_t read_temp_right_zone()
{
	return hvac_values[1].value;
}

static uint8_t read_temp_left_led()
{
	return hvac_values[5].value;
}

static uint8_t read_temp_right_led()
{
	return hvac_values[6].value;
}

static uint8_t read_temp()
{
	return (uint8_t)(((int)read_temp_left_zone() + (int)read_temp_right_zone()) >> 1);
}

static uint8_t read_fanspeed()
{
	return hvac_values[3].value;
}

/* 
 * @param: None
 *
 * @brief: Parse JSON configuration file for blinkm path
 */
static int parse_config()
{
	struct json_object *ledtemp = NULL;
	struct json_object *jobj = json_object_from_file("/etc/hvac.json");

	// Check if .json file has been parsed as a json_object
	if (!jobj) {
		AFB_ERROR("JSON file could not be opened!\n");
		return 1;
	}

	// Check if json_object with key "ledtemp" has been found in .json file
	if (!json_object_object_get_ex(jobj, "ledtemp", &ledtemp)){
		AFB_ERROR("Key not found!\n");
		return 1;
	}

	// Extract directory paths for each LED colour
	json_object_object_foreach(ledtemp, key, value) {
		if (strcmp(key, "red") == 0) {
			led_paths_values.red = json_object_get_string(value);
		} else if (strcmp(key, "green") == 0) {
			led_paths_values.green = json_object_get_string(value);
		} else if (strcmp(key, "blue") == 0) {
			led_paths_values.blue = json_object_get_string(value);
		}
	}

	// return 0 if all succeeded
	return 0;

}

/* 
 * @brief Writing to LED for both temperature sliders
 */
static int temp_write_led()
{

	int rc = -1, red_value, green_value, blue_value;
	int right_temp;
	int left_temp;

	left_temp = read_temp_left_led() - 15;
	right_temp = read_temp_right_led() - 15;

	// Calculates average colour value taken from the temperature toggles
	red_value = (degree_colours[left_temp].rgb[0] + degree_colours[right_temp].rgb[0]) / 2;
	green_value = (degree_colours[left_temp].rgb[1] + degree_colours[right_temp].rgb[1]) / 2;
	blue_value = (degree_colours[left_temp].rgb[2] + degree_colours[right_temp].rgb[2]) / 2;
	
	// default path: /sys/class/leds/blinkm-3-9-red/brightness
	FILE* r = fopen(led_paths_values.red, "w");
	if(r){
		fprintf(r, "%d", red_value);
		fclose(r);
	} else {
		AFB_ERROR("Unable to open red LED path!\n");
		return -1;
	}

	// default path: /sys/class/leds/blinkm-3-9-green/brightness
	FILE* g = fopen(led_paths_values.green, "w");
	if(g){
		fprintf(g, "%d", green_value);
		fclose(g);
	} else {
		AFB_ERROR("Unable to open green LED path!\n");
		return -1;
	}

	// default path: /sys/class/leds/blinkm-3-9-blue/brightness
	FILE* b = fopen(led_paths_values.blue, "w");
	if(b){
		fprintf(b, "%d", blue_value);
		fclose(b);
	} else {
		AFB_ERROR("Unable to open blue LED path!\n");
		return -1;
	}

	return 0;
}

/*
 * @brief Get temperature of left toggle in HVAC system
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void temp_left_zone_led(struct afb_req request)
{
	int i = 5, rc, x, changed;
	double d;
	struct json_object *query, *val;
	uint8_t values[sizeof hvac_values / sizeof *hvac_values];
	uint8_t saves[sizeof hvac_values / sizeof *hvac_values];

	AFB_WARNING("In temp_left_zone_led.");

	query = afb_req_json(request);

	/* records initial values */
	AFB_WARNING("Records initial values");
	values[i] = saves[i] = hvac_values[i].value;


	if (json_object_object_get_ex(query, hvac_values[i].name, &val))
	{
		AFB_WARNING("Value of values[i] = %d", values[i]);
		AFB_WARNING("We got it. Tests if it is an int or double.");
		if (json_object_is_type(val, json_type_int)) {
			x = json_object_get_int(val);
			AFB_WARNING("We get an int: %d",x);
		}
		else if (json_object_is_type(val, json_type_double)) {
			d = json_object_get_double(val);
			x = (int)round(d);
			AFB_WARNING("We get a double: %f => %d",d,x);
		}
		else {
			afb_req_fail_f(request, "bad-request",
				"argument '%s' isn't integer or double", hvac_values[i].name);
			return;
		}
		if (x < 0 || x > 255)
		{
			afb_req_fail_f(request, "bad-request",
				"argument '%s' is out of bounds", hvac_values[i].name);
			return;
		}
		if (values[i] != x) {
			values[i] = (uint8_t)x;
			changed = 1;
			AFB_WARNING("%s changed to %d", hvac_values[i].name,x);
		}
	}
	else {
		AFB_WARNING("%s not found in query!",hvac_values[i].name);
	}


	if (changed) {
		hvac_values[i].value = values[i]; // update structure at line 102
		AFB_WARNING("WRITE_LED: value: %d", hvac_values[i].value);
		rc = temp_write_led();
		if (rc >= 0)
			afb_req_success(request, NULL, NULL);
		else if (retry(temp_write_led)) {
			/* restore initial values */
			hvac_values[i].value = saves[i];
			afb_req_fail(request, "error", "I2C error");
		}
	}
}

/*
 * @brief Get temperature of right toggle in HVAC system
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void temp_right_zone_led(struct afb_req request)
{
	int i = 6, rc, x, changed;
	double d;
	struct json_object *query, *val;
	uint8_t values[sizeof hvac_values / sizeof *hvac_values];
	uint8_t saves[sizeof hvac_values / sizeof *hvac_values];

	AFB_WARNING("In temp_right_zone_led.");

	query = afb_req_json(request);

	/* records initial values */
	AFB_WARNING("Records initial values");
	values[i] = saves[i] = hvac_values[i].value;


	if (json_object_object_get_ex(query, hvac_values[i].name, &val))
	{
		AFB_WARNING("Value of values[i] = %d", values[i]);
		AFB_WARNING("We got it. Tests if it is an int or double.");
		if (json_object_is_type(val, json_type_int)) {
			x = json_object_get_int(val);
			AFB_WARNING("We get an int: %d",x);
		}
		else if (json_object_is_type(val, json_type_double)) {
			d = json_object_get_double(val);
			x = (int)round(d);
			AFB_WARNING("We get a double: %f => %d",d,x);
		}
		else {
			afb_req_fail_f(request, "bad-request",
				"argument '%s' isn't integer or double", hvac_values[i].name);
			return;
		}
		if (x < 0 || x > 255)
		{
			afb_req_fail_f(request, "bad-request",
				"argument '%s' is out of bounds", hvac_values[i].name);
			return;
		}
		if (values[i] != x) {
			values[i] = (uint8_t)x;
			changed = 1;
			AFB_WARNING("%s changed to %d", hvac_values[i].name,x);
		}
	}
	else {
		AFB_WARNING("%s not found in query!", hvac_values[i].name);
	}


	if (changed) {
		hvac_values[i].value = values[i]; // update structure at line 102
		AFB_WARNING("WRITE_LED: value: %d", hvac_values[i].value);
		rc = temp_write_led();
		if (rc >= 0)
			afb_req_success(request, NULL, NULL);
		else if (retry(temp_write_led)) {
			/* restore initial values */
			hvac_values[i].value = saves[i];
			afb_req_fail(request, "error", "I2C error");
		}
	}
}

static int write_can()
{
	struct can_frame txCanFrame;
	int rc = 0;

	rc = can_handler.socket;
	if (rc >= 0)
	{
		// Hardcoded can_id and dlc (data lenght code)
		txCanFrame.can_id = 0x30;
		txCanFrame.can_dlc = 8;
		txCanFrame.data[0] = to_can_temp(read_temp_left_zone());
		txCanFrame.data[1] = to_can_temp(read_temp_right_zone());
		txCanFrame.data[2] = to_can_temp(read_temp());
		txCanFrame.data[3] = 0xf0;
		txCanFrame.data[4] = read_fanspeed();
		txCanFrame.data[5] = 1;
		txCanFrame.data[6] = 0;
		txCanFrame.data[7] = 0;

		AFB_DEBUG("%s: %d %d [%02x %02x %02x %02x %02x %02x %02x %02x]\n",
			can_handler.send_msg,
			txCanFrame.can_id, txCanFrame.can_dlc,
			txCanFrame.data[0], txCanFrame.data[1], txCanFrame.data[2], txCanFrame.data[3],
			txCanFrame.data[4], txCanFrame.data[5], txCanFrame.data[6], txCanFrame.data[7]);

		if(!can_handler.simulation)
		{
			rc = (int)sendto(can_handler.socket, &txCanFrame, sizeof(struct can_frame), 0,
				(struct sockaddr*)&can_handler.txAddress, sizeof(can_handler.txAddress));
			if (rc < 0)
			{
				AFB_ERROR("Sending CAN frame failed.");
			}
		}
	}
	else
	{
		AFB_ERROR("socket not initialized. Attempt to reopen can device socket.");
		open_can_dev();
	}
	return rc;
}

/*****************************************************************************************/
/*****************************************************************************************/
/**											**/
/**											**/
/**	   SECTION: BINDING VERBS IMPLEMENTATION					**/
/**											**/
/**											**/
/*****************************************************************************************/
/*****************************************************************************************/

/*
 * @brief Get fan speed HVAC system
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void get_fanspeed(struct afb_req request)
{
	json_object *ret_json;
	uint8_t fanspeed = read_fanspeed();

	ret_json = json_object_new_object();
	json_object_object_add(ret_json, "FanSpeed", json_object_new_int(fanspeed));

	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Read Consign right zone temperature for HVAC system
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void get_temp_right_zone(struct afb_req request)
{
	json_object *ret_json;
	uint8_t temp = read_temp_right_zone();

	ret_json = json_object_new_object();
	json_object_object_add(ret_json, "RightTemperature", json_object_new_int(temp));

	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Read Consign left zone temperature for HVAC system
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void get_temp_left_zone(struct afb_req request)
{
	json_object *ret_json;
	uint8_t temp = read_temp_left_zone();

	ret_json = json_object_new_object();
	json_object_object_add(ret_json, "LeftTemperature", json_object_new_int(temp));

	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Read all values
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void get(struct afb_req request)
{
	AFB_DEBUG("Getting all values");
	json_object *ret_json;

	ret_json = json_object_new_object();
	json_object_object_add(ret_json, "LeftTemperature", json_object_new_int(read_temp_left_zone()));
	json_object_object_add(ret_json, "RightTemperature", json_object_new_int(read_temp_right_zone()));
	json_object_object_add(ret_json, "FanSpeed", json_object_new_int(read_fanspeed()));

	afb_req_success(request, ret_json, NULL);
}

/*
 * @brief Set a component value using a json object retrieved from request
 *
 * @param struct afb_req : an afb request structure
 *
 */
static void set(struct afb_req request)
{
	int i, rc, x, changed;
	double d;
	struct json_object *query, *val;
	uint8_t values[sizeof hvac_values / sizeof *hvac_values];
	uint8_t saves[sizeof hvac_values / sizeof *hvac_values];

	/* records initial values */
	AFB_DEBUG("Records initial values");
	i = (int)(sizeof hvac_values / sizeof *hvac_values);
	while (i) {
		i--;
		values[i] = saves[i] = hvac_values[i].value;
	}

	/* Loop getting arguments */
	query = afb_req_json(request);
	changed = 0;
	i = (int)(sizeof hvac_values / sizeof *hvac_values);
	AFB_DEBUG("Looping for args. i: %d", i);
	while (i)
	{
		i--;
		AFB_DEBUG("Searching... query: %s, i: %d, comp: %s", json_object_to_json_string(query), i, hvac_values[i].name);
		if (json_object_object_get_ex(query, hvac_values[i].name, &val))
		{
			AFB_DEBUG("We got it. Tests if it is an int or double.");
			if (json_object_is_type(val, json_type_int)) {
				x = json_object_get_int(val);
				AFB_DEBUG("We get an int: %d",x);
			}
			else if (json_object_is_type(val, json_type_double)) {
				d = json_object_get_double(val);
				x = (int)round(d);
				AFB_DEBUG("We get a double: %f => %d",d,x);
			}
			else {
				afb_req_fail_f(request, "bad-request",
					"argument '%s' isn't integer or double", hvac_values[i].name);
				return;
			}
			if (x < 0 || x > 255)
			{
				afb_req_fail_f(request, "bad-request",
					"argument '%s' is out of bounds", hvac_values[i].name);
				return;
			}
			if (values[i] != x) {
				values[i] = (uint8_t)x;
				changed = 1;
				AFB_DEBUG("%s changed to %d",hvac_values[i].name,x);
			}
		}
		else {
			AFB_DEBUG("%s not found in query!",hvac_values[i].name);
		}
	}

	/* attemps to set new values */
	AFB_DEBUG("Diff: %d", changed);
	if (changed)
	{
		i = (int)(sizeof hvac_values / sizeof *hvac_values);
		while (i) {
			i--;
			hvac_values[i].value = values[i];
		}
		rc = write_can();
		if (rc >= 0)
			afb_req_success(request, NULL, NULL);
		else if (retry(write_can)) {
			/* restore initial values */
			i = (int)(sizeof hvac_values / sizeof *hvac_values);
			while (i) {
				i--;
				hvac_values[i].value = saves[i];
			}
			afb_req_fail(request, "error", "CAN error");
		}
	}
	else {
		afb_req_success(request, NULL, "No changes");
	}
}

int bindingServicePreInit(struct afb_service service)
{
	return open_can_dev();
}

int bindingServiceInit(struct afb_service service)
{
	event = afb_daemon_make_event("language");
	if(parse_config() != 0)
		AFB_WARNING("Default values are being used!\n");
	if(afb_daemon_require_api("identity", 1))
		return -1;
	return afb_service_call_sync("identity", "subscribe", NULL, NULL);
}

void onEvent(const char *event_name, struct json_object *object)
{
	json_object *args, *language = json_object_new_object();
	json_object *id_evt_name, *current_identity;

	AFB_NOTICE("Event '%s' received: %s", event_name,
		json_object_to_json_string_ext(object, JSON_C_TO_STRING_PRETTY));

	if (json_object_object_get_ex(object, "eventName", &id_evt_name) &&
	  !strcmp(json_object_get_string(id_evt_name), "login") &&
	  !afb_service_call_sync("identity", "get", NULL, &current_identity)) {
		json_object *response;
		if (! json_object_object_get_ex(current_identity, "response", &response) || ! json_object_object_get_ex(response, "graphPreferredLanguage", &language)) {
			language = json_object_new_string("en_US");
		}
		afb_event_broadcast(event, language);
	}
}

// TODO: Have to change session management flag to AFB_SESSION_CHECK to use token auth
static const struct afb_verb_v2 _afb_verbs_v2_hvac[]= {
	{
		.verb = "get_temp_left_zone",
		.callback = get_temp_left_zone,
		.auth = NULL,
		.info = "Get the left zone temperature",
		.session = AFB_SESSION_NONE_V2
	},
	{
		.verb = "get_temp_right_zone",
		.callback = get_temp_right_zone,
		.auth = NULL,
		.info = "Get the right zone temperature",
		.session = AFB_SESSION_NONE_V2
	},
	{
		.verb = "get_fanspeed",
		.callback = get_fanspeed,
		.auth = NULL,
		.info = "Read fan speed",
		.session = AFB_SESSION_NONE_V2
	},
	{
		.verb = "get",
		.callback = get,
		.auth = NULL,
		.info = "Read all speed",
		.session = AFB_SESSION_NONE_V2
	},
	{
		.verb = "set",
		.callback = set,
		.auth = NULL,
		.info = "Set a HVAC component value",
		.session = AFB_SESSION_NONE_V2
	},
	{
		.verb = "temp_left_zone_led",
		.callback = temp_left_zone_led,
		.auth = NULL,
		.info = "Turn on LED on left temperature zone",
		.session = AFB_SESSION_NONE_V2
	},
	{
		.verb = "temp_right_zone_led",
		.callback = temp_right_zone_led,
		.auth = NULL,
		.info = "Turn on LED on left temperature zone",
		.session = AFB_SESSION_NONE_V2

	},
	{
		.verb = NULL,
		.callback = NULL,
		.auth = NULL,
		.info = NULL,
		.session = 0
	}
};

const struct afb_binding_v2 afbBindingV2 = {
    .api = "hvac",
    .specification = NULL,
    .info = "HVAC service",
    .verbs = _afb_verbs_v2_hvac,
    .preinit = bindingServicePreInit,
    .init = bindingServiceInit,
    .onevent = onEvent,
    .noconcurrency = 0
};