#include "serialport.h"

#include <QSerialPort>

SerialPort::SerialPort(QObject *parent)
	: QObject(parent)
	, m_loopback(false)
	, m_isOpen(false)
	, m_port(new QSerialPort(this))
{
	connect(m_port, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
			[this](QSerialPort::SerialPortError error) {
		if (m_loopback || !m_isOpen)
			return;

		if (error != QSerialPort::SerialPortError::NoError) {
			if (m_port->isOpen())
				m_port->close();
			m_isOpen = false;
			emit errorOccurred("Serial port error: " + m_port->errorString());
			emit closed();
		}
	});
	connect(m_port, &QSerialPort::bytesWritten, this, &SerialPort::bytesWritten);
}

QString SerialPort::errorString() const
{
	if (m_loopback)
		return QString();
	else
		return m_port->errorString();
}

QString SerialPort::portName() const
{
	return m_portName;
}

bool SerialPort::isOpen() const
{
	return m_isOpen;
}

QSerialPort::PinoutSignals SerialPort::pinoutSignals() const
{
	return m_port->pinoutSignals();
}

bool SerialPort::isDataTerminalReady() const
{
	return m_port->isDataTerminalReady();
}

bool SerialPort::isRequestToSend() const
{
	return m_port->isRequestToSend();
}

bool SerialPort::setDataTerminalReady(bool dtr)
{
	return m_port->setDataTerminalReady(dtr);
}

bool SerialPort::setRequestToSend(bool rts)
{
	return m_port->setRequestToSend(rts);
}

void SerialPort::setLoopback(bool loopback)
{
	m_loopback = loopback;
	if (loopback)
		m_portName = "Loopback";
}

void SerialPort::setPortName(const QString &portName)
{
	m_portName = portName;
	m_port->setPortName(portName);
}

void SerialPort::setBaudRate(int baudRate)
{
	m_port->setBaudRate(baudRate);
}

void SerialPort::setDataBits(const QString &dataBits)
{
	m_port->setDataBits(QSerialPort::DataBits(dataBits.toInt()));
}

void SerialPort::setParity(const QString &parity)
{
	static const QSerialPort::Parity pValues[5] = {QSerialPort::EvenParity,
												  QSerialPort::MarkParity,
												  QSerialPort::OddParity,
												  QSerialPort::SpaceParity,
												  QSerialPort::NoParity};
	static const QStringList pStrings = {"even", "mark", "odd", "space", "none"};
	for (int i = 0; i < pStrings.size(); ++i) {
		if (parity.toLower() == pStrings[i]) {
			m_port->setParity(pValues[i]);
			break;
		}
	}
}

void SerialPort::setStopBits(const QString &stopBits)
{
	static const QSerialPort::StopBits sbValues[3] = {QSerialPort::OneStop,
													  QSerialPort::OneAndHalfStop,
													  QSerialPort::TwoStop};
	static const QStringList sbStrings = {"1", "1.5", "2"};
	for (int i = 0; i < sbStrings.size(); ++i) {
		if (stopBits.toLower() == sbStrings[i]) {
			m_port->setStopBits(sbValues[i]);
			break;
		}
	}
}

bool SerialPort::open()
{
	bool ok = m_loopback ? true : m_port->open(QIODevice::ReadWrite);

	if (ok)
		emit opened();
	else
		emit errorOccurred("Failed to open port: " + m_port->errorString());

	m_isOpen = ok;
	return ok;
}

void SerialPort::close()
{
	m_isOpen = false;
	if (!m_loopback)
		m_port->close();
	emit closed();
}

void SerialPort::writeRawData(const QByteArray &data)
{
	if (m_loopback) {
		emit dataRead(data);
		emit bytesWritten(data.size());
	} else {
		m_port->write(data);
	}
}

void SerialPort::writeFormattedData(const QString &data)
{
	QByteArray output;
	for (int i = 0; i < data.size(); ++i) {
		QChar c = data[i];
		char newChar = c.toLatin1();
		if (c == '\\' && i + 1 < data.size()) {
			if (data[i + 1] == '\\') {
				newChar = '\\';
				++i;
			} else if (data[i + 1] == 'n') {
				newChar = '\n';
				++i;
			} else if (data[i + 1] == 'r') {
				newChar = '\r';
				++i;
			} else if (data[i + 1] == 't') {
				newChar = '\t';
				++i;
			} else if (data[i + 1] == 'x' && i + 3 < data.size() &&
					 ((data[i + 2] >= '0' && data[i + 2] <= '9') ||
					  (data[i + 2].toLower() >= 'a' && data[i + 2].toLower() <= 'f')) &&
					 ((data[i + 3] >= '0' && data[i + 3] <= '9') ||
					  (data[i + 3].toLower() >= 'a' && data[i + 3].toLower() <= 'f'))) {
				newChar = char(QString(data[i + 2]).toInt(nullptr, 16) * 16 +
						QString(data[i + 3]).toInt(nullptr, 16));
						i += 3;
			}
		}
		output.append(newChar);
	}
	writeRawData(output);
}
