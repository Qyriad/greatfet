/*
 * This file is part of GreatFET
 *
 * Code for ULPI interfacing, for e.g. USB analysis.
 */

#include <debug.h>

#include <drivers/comms.h>
#include <drivers/gpio.h>
#include <drivers/sgpio.h>
#include <toolchain.h>


#include "../pin_manager.h"
#include "../usb_streaming.h"
#include "../rhododendron.h"

#define CLASS_NUMBER_SELF (0x113)

// For debug only.
extern sgpio_t ulpi_register_mode;


static int ulpi_write_with_retries(uint8_t address, uint8_t data)
{
	int rc;
	uint8_t retries = 128;

	while (retries--) {
		rc = ulpi_register_write(address, data);
		if (rc == 0)  {
			return 0;
		}

		delay_us(1000);
	}

	pr_warning("Failed to write addr %02x := %02x after many retries.\n", address, data);
	return EIO;
}


static int verb_initialize(struct command_transaction *trans)
{
	int rc;

	(void)trans;

	rhododendron_turn_off_led(LED_STATUS);

	// Set up the Rhododendron board for basic use.
	rc = initialize_rhododendron();
	if (rc) {
		return rc;
	}

	delay_us(100000);


	// Swap D+ and D-.
	rc = ulpi_write_with_retries(0x3a, 0b10);
	if (rc) {
		return rc;
	}

	// Disable OTG pulldowns.
	rc = ulpi_write_with_retries(0x0A, 0);
	if (rc) {
		return rc;
	}

	// FIXME: set this to pull up D+ as a demo
	rc = ulpi_write_with_retries(0x04, 0b01001000);
	if (rc) {
		return rc;
	}

	rhododendron_turn_on_led(LED_STATUS);

	// Finally, tell the host how to read data from the capture pipe.
	comms_response_add_uint32_t(trans, USB_STREAMING_BUFFER_SIZE);
	comms_response_add_uint8_t(trans,  USB_STREAMING_IN_ADDRESS);

	return rc;
}


static int verb_ulpi_register_write(struct command_transaction *trans)
{
	uint8_t address = comms_argument_parse_uint8_t(trans);
	uint8_t value   = comms_argument_parse_uint8_t(trans);

	if (!comms_argument_parse_okay(trans)) {
		return EBADMSG;
	}

	return ulpi_register_write(address, value);
}


static int verb_dump_register_sgpio_config(struct command_transaction *trans)
{
	bool include_unused = comms_argument_parse_bool(trans);

	if (!comms_transaction_okay(trans)) {
		return EINVAL;
	}

	sgpio_dump_configuration(LOGLEVEL_INFO, &ulpi_register_mode, include_unused);
	return 0;
}


static int verb_start_capture(struct command_transaction *trans)
{
	return rhododendron_start_capture();

}

static int verb_stop_capture(struct command_transaction *trans)
{
	(void)trans;
	rhododendron_stop_capture();

	return 0;
}


static struct comms_verb _verbs[] = {

		// Control.
		{  .name = "initialize", .handler = verb_initialize, .in_signature = "",
			.out_signature = "<IB", .out_param_names = "buffer_size, endpoint",
			.doc = "configures the target Rhododendendron board for capture (and pass-through)" },

		{  .name = "start_capture", .handler = verb_start_capture, .in_signature = "", .out_signature = "",
           .doc = "starts a capture of high-speed USB data" },
		{  .name = "stop_capture", .handler = verb_stop_capture, .in_signature = "", .out_signature = "",
           .doc = "halts the active USB capture" },


		// Debug.
		{  .name = "ulpi_register_write", .handler = verb_ulpi_register_write, .in_signature = "<BB",
		   .out_signature = "", .in_param_names = "register_address, register_value", .out_param_names = "",
           .doc = "debug: write directly to a register on the attached USB phy"},
		{ .name = "dump_register_sgpio_configuration",  .handler = verb_dump_register_sgpio_config,
			.in_signature = "<?", .out_signature="", .in_param_names = "include_unused",
			.doc = "Requests that the system dumps its SGPIO configuration state to the debug ring." },

		// Sentinel.
		{}
};
COMMS_DEFINE_SIMPLE_CLASS(usb_analyzer, CLASS_NUMBER_SELF, "usb_analyzer", _verbs,
        "functionality for analyzing USB");
