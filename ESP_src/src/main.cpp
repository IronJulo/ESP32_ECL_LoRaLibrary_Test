#include <Arduino.h>

#include <LoRaDevice.h>
#include <SPI.h>

#include "Transport.h"
#include "Transaction.h"
#include "AbstractGpio.h"

class ESP_32_SPI : public ecl::Transport
{
public:
	bool m_locked = false;
	SPIClass &m_spi;
	SPISettings m_spiSettings;
	int m_miso;
	int m_mosi;
	int m_ss;
	int m_sck;
	int m_rst;
	int m_dio;

public:
	ESP_32_SPI(SPIClass &spi, int miso, int mosi, int ss, int sck, int rst, int dio) : ecl::Transport(ecl::Endianness::BIG),
																					   m_spiSettings(8E6, MSBFIRST, SPI_MODE0),
																					   m_spi(spi),
																					   m_miso(miso),
																					   m_mosi(mosi),
																					   m_ss(ss),
																					   m_sck(sck),
																					   m_rst(rst),
																					   m_dio(rst)
	{
	}

	~ESP_32_SPI()
	{
	}

	bool lock() override
	{
		digitalWrite(m_ss, LOW);
		m_spi.beginTransaction(m_spiSettings);
		m_locked = true;
		return m_locked;
	}

	bool unlock() override
	{
		digitalWrite(m_ss, HIGH);
		m_spi.endTransaction();
		m_locked = false;
		return m_locked;
	}

	void write(uint8_t data) override
	{
		m_spi.transfer(data);
	}

	uint8_t read() override
	{
		return m_spi.transfer(0x00);
	}

	uint8_t transfer(uint8_t data) override
	{
		return m_spi.transfer(data);
	}

	void begin() override
	{
		// Calling multiple time begin should be OK TODO: researsh on this
		m_spi.begin(m_sck, m_miso, m_mosi, m_ss);
	}

	ecl::Transaction startTransaction() override
	{
		// How can this not causes compile errors ?????
		// return ecl::Transaction(static_cast<ecl::Transaction>(*this));
		return ecl::Transaction(*this);
	}
};

class ESP_32_GPIO : public ecl::AbstractGpio
{
private:
	int m_pinNumber;

public:
	ESP_32_GPIO(int pinNumber) : m_pinNumber(pinNumber)
	{
	}

	void setDirection(ecl::Gpio::Direction direction)
	{
		ecl::AbstractGpio::setDirection(direction);
		switch (direction)
		{
		case ecl::Gpio::Direction::Input:
			pinMode(m_pinNumber, INPUT);
			break;
		case ecl::Gpio::Direction::Output:
			pinMode(m_pinNumber, OUTPUT);
			break;
		default: // TODO Throw
			break;
		}
	}

	void setResistor(ecl::Gpio::Resistor resistor)
	{
		ecl::AbstractGpio::setResistor(resistor);
	}
	void setState(ecl::Gpio::State state)
	{
		ecl::AbstractGpio::setState(state);
		switch (state)
		{
		case ecl::Gpio::State::High:
			digitalWrite(m_pinNumber, HIGH);
			break;
		case ecl::Gpio::State::Low:
			digitalWrite(m_pinNumber, LOW);
			break;
		default: // TODO Throw
			break;
		}
	}

	void configureInterrupt(ecl::Gpio::InterruptEdge edge, ecl::Gpio::InterruptCallback callback)
	{
		ecl::AbstractGpio::configureInterrupt(edge, callback);

		int mode = -1;
		switch (edge)
		{
		case ecl::Gpio::InterruptEdge::Rising:
			mode = RISING;
			break;
		case ecl::Gpio::InterruptEdge::Falling:
			mode = FALLING;
			break;
		case ecl::Gpio::InterruptEdge::OnLow:
			mode = ONLOW;
			break;
		case ecl::Gpio::InterruptEdge::OnHigh:
			mode = ONHIGH;
			break;
		case ecl::Gpio::InterruptEdge::OnLowWe:
			mode = ONLOW_WE;
			break;
		case ecl::Gpio::InterruptEdge::OnHighWe:
			mode = ONHIGH_WE;
			break;
		default: // TODO Throw
			break;
		}

		attachInterrupt(m_pinNumber, callback, mode);
	};
};

SPIClass spiClass(0);
ESP_32_SPI espSpi(spiClass, 7, 6, 2, 3, 0, 1);
ESP_32_GPIO resetGpio(0);
ESP_32_GPIO dio0Gpio(1);
LoRaDevice loraDevice(espSpi, resetGpio, dio0Gpio);

const double frequency = 433E6;

void setup()
{
	Serial.begin(9600);
	while (!Serial)
		;

	Serial.println("-----------------------------------------------------------------");
	Serial.flush();
	delay(1000);
	Serial.println("");
	Serial.println("test");

	LoRaError error = loraDevice.init(frequency);

	if (error.isNot(LoRaError::OK))
	{
		Serial.println("Lora init failed");
		Serial.println(error.toString().c_str());
		while (true)
			;
	}
}

void loop()
{
	Serial.println("Waiting");
	delay(100);
}