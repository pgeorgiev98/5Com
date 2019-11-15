#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QComboBox;
class QScrollArea;
class QSpinBox;
class HexView;
class PlainTextView;
class QPushButton;
class SerialPort;
class ByteReceiveTimesDialog;
class ContinuousSendWindow;
class SendSequenceWindow;
class InputField;
class PreferencesPage;
class FontPreferencesPage;
class QAction;
class QShortcut;

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
	void showPinoutSignals();
	void showByteReceiveTimes();
	void showChangelog();
	void showLicense();
	void showAboutPage();
	void showAboutQtPage();
	void showCheckForUpdates();
	void showPreferences();
	void showFontPreferences();
	void showKeyboardShortcutsDialog();
	void sendFromFile();
	void exportData();
	void clearScreen();
	void refreshStatusBar();
	void trimData();
	void updateKeyboardShortcuts();

protected:
	void resizeEvent(QResizeEvent *) override;
	void closeEvent(QCloseEvent *event) override;

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

	QTabWidget *m_tabs;
	QSpinBox *m_hexViewBytesPerLine;

	InputField *m_inputField;
	PlainTextView *m_textView;
	HexView *m_hexView;
	QScrollArea *m_hexViewScrollArea;
	QScrollArea *m_plainTextViewScrollArea;

	SerialPort *m_port;

	ContinuousSendWindow *m_continuousSendWindow;
	SendSequenceWindow *m_sendSequenceWindow;
	QDialog *m_pinoutSignalsDialog;
	ByteReceiveTimesDialog *m_byteReceiveTimesDialog;
	QDialog *m_asciiTableDialog;
	QDialog *m_escapeCodesDialog;
	PreferencesPage *m_preferencesPage;
	FontPreferencesPage *m_fontPreferencesPage;

	QAction *m_exportAction;
	QAction *m_clearScreenAction;
	QAction *m_writeFileAction;
	QAction *m_quitAction;

	QShortcut *m_focusInputShortcut;
	QShortcut *m_clearInputShortcut;
	QShortcut *m_plainTextViewShortcut;
	QShortcut *m_hexViewShortcut;
};

#endif // MAINWINDOW_H
