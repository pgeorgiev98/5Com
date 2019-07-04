#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QSerialPort>

class SerialPort : public QObject
{
	Q_OBJECT
public:
	explicit SerialPort(QObject *parent = nullptr);
	QString errorString() const;
	QString portName() const;
	bool isOpen() const;
	QSerialPort::PinoutSignals pinoutSignals() const;
	bool isDataTerminalReady() const;
	bool isRequestToSend() const;
	bool setDataTerminalReady(bool dtr);
	bool setRequestToSend(bool rts);

signals:
	void opened();
	void closed();
	void dataRead(const QByteArray &data);
	void bytesWritten(int bytes);
	void errorOccurred(const QString &errorString);

public slots:
	void setLoopback(bool loopback);
	void setPortName(const QString &portName);
	void setBaudRate(int baudRate);
	void setDataBits(const QString &dataBits);
	void setParity(const QString &parity);
	void setStopBits(const QString &stopBits);

	bool open();
	void close();

	void writeRawData(const QByteArray &data);
	void writeFormattedData(const QString &data);

private slots:
	void readFromPort();

private:
	bool m_loopback;
	bool m_isOpen;
	QString m_portName;

	QSerialPort *m_port;
};

#endif // SERIALPORT_H
