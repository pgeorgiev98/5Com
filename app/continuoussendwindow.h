#ifndef CONTINUOUSSENDWINDOW_H
#define CONTINUOUSSENDWINDOW_H

#include <QDialog>

class SerialPort;

class QTimer;

class QStackedLayout;

class QLineEdit;
class QCheckBox;
class QSpinBox;
class QLabel;

class ContinuousSendWindow : public QDialog
{
	Q_OBJECT
public:
	explicit ContinuousSendWindow(SerialPort *port, QWidget *parent = nullptr);

public slots:
	void setInput(const QString &input);

private slots:
	void startSending();
	void stopSending();
	void sendPacket();

private:
	int m_packetsSent;
	SerialPort *m_port;
	QTimer *m_sendTimer;

	QStackedLayout *m_layout;

	QLineEdit *m_input;
	QSpinBox *m_interval;
	QCheckBox *m_sendIndefinitely;
	QSpinBox *m_packetCount;
	QLabel *m_packetsSendLabel;
};

#endif // CONTINUOUSSENDWINDOW_H
