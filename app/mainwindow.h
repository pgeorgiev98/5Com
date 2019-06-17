#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QComboBox;
class QLineEdit;
class HexView;
class PlainTextView;
class QPushButton;
class QSerialPort;
class ByteReceiveTimesDialog;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void sendRawData(const QByteArray &data);
	void sendData(const QString &data);
	void sendDataFromMainInput();
	bool openPort();
	void closePort();
	void displayData(const QByteArray &data);
	void toggleConnect();
	void readFromPort();
	void refreshPorts();

signals:
	void bytesWritten(qint64 bytes);

public slots:
	void continuousSend();
	void showAsciiTable();
	void showEscapeCodes();
	void showPinoutSignals();
	void showByteReceiveTimes();
	void showCheckForUpdates();
	void showChangelog();
	void showLicense();
	void showAboutPage();
	void showAboutQtPage();
	void showSettings();
	void sendFromFile();
	void exportData();
	void clearScreen();
	void refreshStatusBar();
	void trimData();

private:
	qint64 m_totalBytesRead;
	qint64 m_totalBytesWritten;
	QByteArray m_receivedData;
	int m_continuousPacketsSent;

	QLabel *m_statusBarLabel;

	QComboBox *m_portSelect;
	QComboBox *m_baudRateSelect;
	QComboBox *m_dataBitsSelect;
	QComboBox *m_paritySelect;
	QComboBox *m_stopBitsSelect;
	QPushButton *m_connectButton;

	QLineEdit *m_inputField;
	PlainTextView *m_textView;
	HexView *m_hexView;

	QSerialPort *m_port;
	bool m_opened;
	bool m_usingLoopback;

	QDialog *m_continuousSendDialog;
	QDialog *m_pinoutSignalsDialog;
	ByteReceiveTimesDialog *m_byteReceiveTimesDialog;
	QDialog *m_asciiTableDialog;
	QDialog *m_escapeCodesDialog;
};

#endif // MAINWINDOW_H
