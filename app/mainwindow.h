#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QComboBox;
class HexView;
class PlainTextView;
class QPushButton;
class SerialPort;
class ByteReceiveTimesDialog;
class ContinuousSendWindow;
class SendSequenceWindow;
class InputField;
class SettingsPage;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);

private slots:
	void sendDataFromInput();
	void closePort();
	void displayData(const QByteArray &data);
	void toggleConnect();
	void refreshPorts();

	void onPortOpened();
	void onPortClosed();

public slots:
	void continuousSend();
	void sendSequence();
	void showAsciiTable();
	void showEscapeCodes();
	void showKeyboardShortcuts();
	void showPinoutSignals();
	void showByteReceiveTimes();
	void showChangelog();
	void showLicense();
	void showAboutPage();
	void showAboutQtPage();
	void showCheckForUpdates();
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

	InputField *m_inputField;
	PlainTextView *m_textView;
	HexView *m_hexView;

	SerialPort *m_port;

	ContinuousSendWindow *m_continuousSendWindow;
	SendSequenceWindow *m_sendSequenceWindow;
	QDialog *m_pinoutSignalsDialog;
	ByteReceiveTimesDialog *m_byteReceiveTimesDialog;
	QDialog *m_asciiTableDialog;
	QDialog *m_escapeCodesDialog;
	SettingsPage *m_settingsPage;
};

#endif // MAINWINDOW_H
