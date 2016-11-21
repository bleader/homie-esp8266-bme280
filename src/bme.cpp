#include <Wire.h>
#include <Homie.h>
#include <SPI.h>
#include <BME280I2C.h>

/* comment this out to remove almost all prints
 * this saves an average of 80ms on a roughly 4s uptime between deep sleeps */
#define DEBUG_MODE 1

/* uncomment to activate battery operated mode, a.k.a deep sleep mode */
#define BATTERY_MODE 1

#ifdef BATTERY_MODE
#define FW_NAME "raton-bme-battery"
#define DEEPSLEEP_TIME (10 * 60 * 1000000) /* 10 minutes */
#else
#define FW_NAME "raton-bme"
#define PUB_INTERVAL 60
#endif

#define FW_VERSION	"0.1.0"

/* Magic sequence for Autodetectable Binary Upload */
const char *__FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" FW_NAME "\x93\x44\x6b\xa7\x75";
const char *__FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" FW_VERSION "\xb0\x30\x48\xd4\x1a";
/* End of magic sequence for Autodetectable Binary Upload */


HomieNode temperatureNode("temperature", "temperature");
HomieNode humidityNode("humidity", "humidity");
HomieNode pressureNode("pressure", "pressure");
HomieNode batteryNode("battery", "battery");

BME280I2C bme;
#define PRESSURE_OFFSET 16

ADC_MODE(ADC_VCC);
unsigned long published = 0;

void setupHandler() {
	temperatureNode.setProperty("unit").send("c");
	humidityNode.setProperty("unit").send("%");
	pressureNode.setProperty("unit").send("hPa");
	batteryNode.setProperty("unit").send("V");

	/* 4 and 5 are the gpio numbers */
	if (!bme.begin(4,5))
		Serial.println("Could not find BME280 sensor, check wiring!");
}

void loopHandler() {
#ifdef BATTERY_MODE
	/* make sure not to send anything until mqtt is connected */
	if (Homie.isConnected() && !published)
#else
	if (millis() - published >= PUB_INTERVAL * 1000UL)
#endif
	{
		float t, h, p, v;

		bme.read(p, t, h, true, 1); /* true for metric, 1 for hPa */
		p += PRESSURE_OFFSET;
		v = ESP.getVcc() / 1000.0f;

#ifdef DEBUG_MODE
		Serial << "t = " << t << "°C p = " << p << "hPa / h = " << h
			<< " % /  v = " << v  << "V" << endl;
#endif

		/* only try to publish if everything seems right */
		if (!isnan(t) && !isnan(h) && !isnan(p) && !isnan(v)) {
			temperatureNode.setProperty("degrees").send(String(t));
			humidityNode.setProperty("relative").send(String(h));
			pressureNode.setProperty("pressure").send(String(p));
			batteryNode.setProperty("battery").send(String(v));

#ifdef BATTERY_MODE
			published = 1;
			Homie.prepareToSleep();
#else
			published = millis();
#endif
		}
	}
}

#ifdef BATTERY_MODE
void onHomieEvent(const HomieEvent& event) {
	switch(event.type) {
		case HomieEventType::READY_TO_SLEEP:
#ifdef DEBUG_MODE
			Serial << "Ready to sleep" << endl;
#endif
			ESP.deepSleep(DEEPSLEEP_TIME);
			break;
 	}
}
#endif

void setup() {
#ifdef DEBUG_MODE
	Serial.begin(115200);
	Serial.println();
	Serial.println();
#else
	Homie.disableLogging();
#endif

	Homie_setFirmware(FW_NAME, FW_VERSION);

	Homie.setSetupFunction(setupHandler);
	Homie.setLoopFunction(loopHandler);
	Homie.disableLedFeedback(); 

#ifdef BATTERY_MODE
	Homie.onEvent(onHomieEvent);
#endif

	Homie.setup();
}

void loop() {
	Homie.loop();
}
