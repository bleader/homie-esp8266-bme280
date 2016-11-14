#include <Wire.h>
#include <Homie.h>
#include <SPI.h>
#include <BME280I2C.h>

#define FW_NAME		"raton-bme"
#define FW_VERSION	"0.0.2"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */

#define PUB_INTERVAL  60

HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");
HomieNode pressureNode("pressure", "pressure");

BME280I2C bme;
#define PRESSURE_OFFSET 16

unsigned long lastPublish = 0;

void setupHandler() {
	temperatureNode.setProperty("unit").send("c");
	humidityNode.setProperty("unit").send("%");
	pressureNode.setProperty("unit").send("hPa");

	/* 4 and 5 are the gpio numbers */
	if (!bme.begin(4,5))
		Serial.println("Could not find BME280 sensor, check wiring!");
}

void loopHandler() {
	if (millis() - lastPublish >= PUB_INTERVAL * 1000UL) {
		float t, h, p;

		bme.read(p, t, h, true, 1); /* true for metric, 1 for hPa */
		p += PRESSURE_OFFSET;

		if (!isnan(t) &&
		    temperatureNode.setProperty("degrees").send(String(t))) {
			lastPublish = millis();
		}
		if (!isnan(h) &&
		    humidityNode.setProperty("relative").send(String(h))) {
			lastPublish = millis();
		}
		if (!isnan(p) &&
		    pressureNode.setProperty("pressure").send(String(p))) {
			lastPublish = millis();
		}
	}
}

void setup() {
	Serial.begin(115200);

	Homie_setFirmware(FW_NAME, FW_VERSION);

	Homie.setSetupFunction(setupHandler);
	Homie.setLoopFunction(loopHandler);

	Homie.disableLedFeedback(); 
	Homie.setup();
}

void loop() {
	Homie.loop();
}
