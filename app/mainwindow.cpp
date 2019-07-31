#include "mainwindow.h"
#include "common.h"
#include "hexview.h"
#include "plaintextview.h"
#include "asciitable.h"
#include "escapecodesdialog.h"
#include "bytereceivetimesdialog.h"
#include "sendfiledialog.h"
#include "latestreleasechecker.h"
#include "changelogdialog.h"
#include "exportdialog.h"
#include "settingspage.h"
#include "config.h"
#include "serialport.h"
#include "continuoussendwindow.h"
#include "sendsequencewindow.h"
#include "inputfield.h"
#include "checkforupdatesdialog.h"
#include "keyboardshortcutswindow.h"

#include <QComboBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialog>
#include <QStatusBar>
#include <QTimer>
#include <QTime>
#include <QProgressBar>
#include <QShortcut>
#include <QScrollArea>
#include <QScrollBar>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_totalBytesRead(0)
	, m_totalBytesWritten(0)
	, m_continuousPacketsSent(0)
	, m_statusBarLabel(new QLabel)
	, m_portSelect(new QComboBox)
	, m_baudRateSelect(new QComboBox)
	, m_dataBitsSelect(new QComboBox)
	, m_paritySelect(new QComboBox)
	, m_stopBitsSelect(new QComboBox)
	, m_connectButton(new QPushButton)
	, m_tabs(new QTabWidget)
	, m_hexViewBytesPerLine(new QSpinBox(m_tabs))
	, m_inputField(new InputField)
	, m_textView(new PlainTextView)
	, m_hexView(new HexView)
	, m_hexViewScrollArea(new QScrollArea)
	, m_plainTextViewScrollArea(new QScrollArea)
	, m_port(new SerialPort(this))
	, m_continuousSendWindow(new ContinuousSendWindow(m_port, this))
	, m_sendSequenceWindow(new SendSequenceWindow(m_port, this))
	, m_pinoutSignalsDialog(nullptr)
	, m_byteReceiveTimesDialog(nullptr)
	, m_asciiTableDialog(new AsciiTable(this))
	, m_escapeCodesDialog(new EscapeCodesDialog(this))
	, m_keyboardShortcutsWindow(new KeyboardShortcutsWindow(this))
	, m_settingsPage(new SettingsPage(this))
{
	Config c;
	setMinimumSize(640, 480);
	setWindowIcon(QIcon(":/icon.ico"));

	bool checkForUpdates = c.checkForUpdatesOnStartup();
	QLabel *updateStatusBarLabel = new QLabel;

	if (checkForUpdates) {
		updateStatusBarLabel->setText("Checking for updates...");
		statusBar()->addWidget(updateStatusBarLabel);
	}

	statusBar()->addPermanentWidget(m_statusBarLabel);

	// Keyboard shortcuts dialog

	m_portSelect->addItem("Loopback");
	for (qint32 baud : QSerialPortInfo::standardBaudRates())
		m_baudRateSelect->addItem(QString::number(baud));
	m_dataBitsSelect->addItems({"5", "6", "7", "8"});
	m_paritySelect->addItems({"Even", "Mark", "Odd", "Space", "None"});
	m_stopBitsSelect->addItems({"1", "1.5", "2"});

	m_baudRateSelect->setCurrentText("9600");
	m_dataBitsSelect->setCurrentText("8");
	m_paritySelect->setCurrentText("None");
	m_stopBitsSelect->setCurrentText("1");

	m_connectButton->setText(" Connect  ");

	m_baudRateSelect->addItem("Add Custom");

	connect(m_portSelect, &QComboBox::currentTextChanged, [this](const QString &text) {
		if (m_portSelect->currentIndex() == 0) {
			m_port->setLoopback(true);
		} else {
			m_port->setLoopback(false);
			m_port->setPortName(text);
		}
	});
	connect(m_baudRateSelect, &QComboBox::currentTextChanged, [this](const QString &text) {
		if (text == "Add Custom") {
			QGridLayout *layout = new QGridLayout;
			QSpinBox *baudInput = new QSpinBox;
			QPushButton *accept = new QPushButton("Accept");
			QPushButton *cancel = new QPushButton("Cancel");
			baudInput->setMinimum(1);
			baudInput->setMaximum(INT_MAX);
			baudInput->setValue(9600);

			QDialog dialog(this);
			dialog.setLayout(layout);
			layout->addWidget(baudInput, 0, 0, 1, 2);
			layout->addWidget(accept, 1, 0);
			layout->addWidget(cancel, 1, 1);

			connect(accept, &QPushButton::clicked, &dialog, &QDialog::accept);
			connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);

			if (dialog.exec()) {
				QString baudString = QString::number(baudInput->value());
				bool contains = false;
				for (int i = 0; i < m_baudRateSelect->count(); ++i) {
					if (m_baudRateSelect->itemText(i) == baudString) {
						contains = true;
						break;
					}
				}
				if (contains) {
					QMessageBox::information(this, APPLICATION_NAME, "Baud rate was already in the list");
				} else {
					m_baudRateSelect->addItem(baudString);
				}
				m_baudRateSelect->setCurrentText(baudString);
			}
		}
		m_port->setBaudRate(m_baudRateSelect->currentText().toInt());
	});
	connect(m_dataBitsSelect, &QComboBox::currentTextChanged, m_port, &SerialPort::setDataBits);
	connect(m_paritySelect, &QComboBox::currentTextChanged, m_port, &SerialPort::setParity);
	connect(m_stopBitsSelect, &QComboBox::currentTextChanged, m_port, &SerialPort::setStopBits);


	QWidget *central = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;
	central->setLayout(layout);
	setCentralWidget(central);

	{
		m_plainTextViewScrollArea->setPalette(m_textView->palette());
		m_plainTextViewScrollArea->setWidget(m_textView);
		m_tabs->addTab(m_plainTextViewScrollArea, "Plain Text View");
	}
	{
		m_hexViewScrollArea->setPalette(m_hexView->palette());
		m_hexViewScrollArea->setWidget(m_hexView);
		m_tabs->addTab(m_hexViewScrollArea, "Hex View");
	}

	m_hexViewBytesPerLine->setSuffix(" bytes per line");
	m_hexViewBytesPerLine->setRange(1, 128);
	m_hexViewBytesPerLine->setValue(c.hexViewBytesPerLine());
	m_hexViewBytesPerLine->adjustSize();
	m_hexViewBytesPerLine->move(this->width() - m_hexViewBytesPerLine->width(), 0);
	m_hexView->setBytesPerLine(m_hexViewBytesPerLine->value());

	{
		QGridLayout *portLayout = new QGridLayout;
		int col = 0;
		portLayout->addWidget(new QLabel("Port: "), 0, col);
		portLayout->addWidget(m_portSelect, 1, col++);
		portLayout->addWidget(new QLabel("Baud Rate: "), 0, col);
		portLayout->addWidget(m_baudRateSelect, 1, col++);
		portLayout->addWidget(new QLabel("Data Bits: "), 0, col);
		portLayout->addWidget(m_dataBitsSelect, 1, col++);
		portLayout->addWidget(new QLabel("Parity: "), 0, col);
		portLayout->addWidget(m_paritySelect, 1, col++);
		portLayout->addWidget(new QLabel("Stop Bits: "), 0, col);
		portLayout->addWidget(m_stopBitsSelect, 1, col++);
		portLayout->addWidget(m_connectButton, 0, col++, 2, 1);

		layout->addLayout(portLayout);
	}

	{
		QPushButton *sendButton = new QPushButton("Send");
		QHBoxLayout *inputLayout = new QHBoxLayout;
		inputLayout->addWidget(m_inputField, 1);
		inputLayout->addWidget(sendButton);

		layout->addLayout(inputLayout);

		connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendDataFromInput);
		connect(m_inputField->lineEdit(), &QLineEdit::returnPressed, this, &MainWindow::sendDataFromInput);
	}

	layout->addWidget(m_tabs);

	connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::toggleConnect);

	QAction *writeFile = new QAction("Write &file to port");
	QAction *exportAction = new QAction("&Export");
	QAction *exitAction = new QAction("E&xit");
	QAction *continuousSendAction = new QAction("&Continuous send");
	QAction *sendSequenceAction = new QAction("Send se&quence");
	QAction *pinoutSignalsAction = new QAction("&Pinout signals");
	QAction *byteReceiveTimesAction = new QAction("&Byte receive times");
	QAction *clearScreenAction = new QAction("C&lear screen");
	QAction *checkForUpdatesAction = new QAction("Check for &updates");
	QAction *settingsAction = new QAction("&Settings");
	QAction *asciiAction = new QAction("ASCII &table");
	QAction *escapeCodesAction = new QAction("&Escape codes");
	QAction *keyboardShortcutsAction = new QAction("&Keyboard shortcuts");
	QAction *changelogAction = new QAction("Change&log");
	QAction *licenseAction = new QAction("&License");
	QAction *aboutQtAction = new QAction("About &Qt");
	QAction *aboutAction = new QAction("&About");

	writeFile->setShortcut(QKeySequence::Open);
	exportAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
	exitAction->setShortcut(QKeySequence::Quit);
	clearScreenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));

	auto fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(writeFile);
	fileMenu->addAction(exportAction);
	fileMenu->addAction(exitAction);

	auto toolsMenu = menuBar()->addMenu("&Tools");
	toolsMenu->addAction(continuousSendAction);
	toolsMenu->addAction(sendSequenceAction);
	toolsMenu->addAction(pinoutSignalsAction);
	toolsMenu->addAction(byteReceiveTimesAction);
	toolsMenu->addAction(clearScreenAction);
	toolsMenu->addAction(checkForUpdatesAction);
	toolsMenu->addAction(settingsAction);

	auto helpMenu = menuBar()->addMenu("&Help");
	helpMenu->addAction(asciiAction);
	helpMenu->addAction(escapeCodesAction);
	helpMenu->addAction(keyboardShortcutsAction);
	helpMenu->addSeparator();
	helpMenu->addAction(changelogAction);
	helpMenu->addAction(licenseAction);
	helpMenu->addAction(aboutQtAction);
	helpMenu->addAction(aboutAction);

	connect(writeFile, &QAction::triggered, this, &MainWindow::sendFromFile);
	connect(exportAction, &QAction::triggered, this, &MainWindow::exportData);
	connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
	connect(continuousSendAction, &QAction::triggered, this, &MainWindow::continuousSend);
	connect(sendSequenceAction, &QAction::triggered, this, &MainWindow::sendSequence);
	connect(pinoutSignalsAction, &QAction::triggered, this, &MainWindow::showPinoutSignals);
	connect(byteReceiveTimesAction, &QAction::triggered, this, &MainWindow::showByteReceiveTimes);
	connect(clearScreenAction, &QAction::triggered, this, &MainWindow::clearScreen);
	connect(checkForUpdatesAction, &QAction::triggered, this, &MainWindow::showCheckForUpdates);
	connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
	connect(asciiAction, &QAction::triggered, this, &MainWindow::showAsciiTable);
	connect(escapeCodesAction, &QAction::triggered, this, &MainWindow::showEscapeCodes);
	connect(keyboardShortcutsAction, &QAction::triggered, this, &MainWindow::showKeyboardShortcuts);
	connect(changelogAction, &QAction::triggered, this, &MainWindow::showChangelog);
	connect(licenseAction, &QAction::triggered, this, &MainWindow::showLicense);
	connect(aboutQtAction, &QAction::triggered, this, &MainWindow::showAboutQtPage);
	connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutPage);

	refreshPorts();
	if (m_portSelect->count() > 1)
		m_portSelect->setCurrentIndex(1);
	if (c.rememberLastUsedPort()) {
		QString lastPort = c.lastUsedPort();
		if (!lastPort.isEmpty())
			m_portSelect->setCurrentText(lastPort);

		m_baudRateSelect->setCurrentText(c.lastUsedPortBaudRate());
		m_dataBitsSelect->setCurrentText(c.lastUsedPortDataBits());
		m_paritySelect->setCurrentText(c.lastUsedPortParity());
		m_stopBitsSelect->setCurrentText(c.lastUsedPortStopBits());
	}

	QTimer *refreshTimer = new QTimer(this);
	refreshTimer->start(2000);
	connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshPorts);

	// Disable some things depending on which port is selected
	auto onSelectedPortChanged = [this, pinoutSignalsAction](int index) {
		bool usingLoopback = (index == 0);
		// Disable the pinout signals action for the Loopback device
		pinoutSignalsAction->setDisabled(usingLoopback);
		// Disable the port settings for the Loopback device
		for (QWidget *w : {m_baudRateSelect, m_dataBitsSelect, m_paritySelect, m_stopBitsSelect})
			w->setDisabled(usingLoopback);
	};
	connect(m_portSelect, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), onSelectedPortChanged);
	onSelectedPortChanged(m_portSelect->currentIndex());

	connect(m_port, &SerialPort::dataRead, this, &MainWindow::displayData);
	connect(m_port, &SerialPort::errorOccurred, [this](const QString &errorString) {
		QMessageBox::critical(this, APPLICATION_NAME, errorString);
	});
	connect(m_port, &SerialPort::opened, this, &MainWindow::onPortOpened);
	connect(m_port, &SerialPort::closed, this, &MainWindow::onPortClosed);

	connect(m_port, &SerialPort::bytesWritten, [this](qint64 bytes) {
		m_totalBytesWritten += bytes;
		refreshStatusBar();
	});

	// Keyboard shortcuts
	QShortcut *focusInputShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_L), this);
	QShortcut *clearInputShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_W), m_inputField);
	QShortcut *plainTextViewShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_1), this);
	QShortcut *hexViewShortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_2), this);
	connect(focusInputShortcut, &QShortcut::activated, [this]() { m_inputField->setFocus(); });
	connect(clearInputShortcut, &QShortcut::activated, [this]() {
		if (m_inputField->hasFocus())
			m_inputField->clearText();
	});
	connect(plainTextViewShortcut, &QShortcut::activated, [this]() { m_tabs->setCurrentIndex(0); });
	connect(hexViewShortcut, &QShortcut::activated, [this]() { m_tabs->setCurrentIndex(1); });

	m_textView->setColorSpecialCharacters(c.colorSpecialCharacters());
	connect(m_settingsPage, &SettingsPage::settingsChanged, [this]() {
		Config c;
		m_textView->clear();
		m_textView->setColorSpecialCharacters(c.colorSpecialCharacters());
		m_textView->insertData(m_receivedData);
		int bytesPerLine = c.hexViewBytesPerLine();
		if (bytesPerLine != m_hexViewBytesPerLine->value())
			m_hexViewBytesPerLine->setValue(bytesPerLine);
	});

	connect(m_hexViewBytesPerLine, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			m_hexView, &HexView::setBytesPerLine);
	connect(m_tabs, &QTabWidget::currentChanged, [this](int index) {
		m_hexViewBytesPerLine->setVisible(index == 1);
	});
	m_hexViewBytesPerLine->setVisible(m_tabs->currentIndex() == 1);

	connect(m_textView, &PlainTextView::highlightInHexView, [this](ByteSelection selection) {
		m_tabs->setCurrentIndex(1);
		QPoint p = m_hexView->getByteCoordinates(selection.begin);
		m_hexViewScrollArea->verticalScrollBar()->setValue(p.y());
		m_hexView->highlight(selection);
	});

	connect(m_hexView, &HexView::highlightInTextView, [this](ByteSelection selection) {
		m_tabs->setCurrentIndex(0);
		QPoint p = m_textView->getByteCoordinates(selection.begin);
		m_plainTextViewScrollArea->verticalScrollBar()->setValue(p.y());
		m_plainTextViewScrollArea->horizontalScrollBar()->setValue(p.x());
		m_textView->highlight(selection);
	});

	connect(m_tabs, &QTabWidget::currentChanged, [this](int tab) {
		if (tab == 0) {
			auto selection = m_hexView->selection();
			if (selection.has_value()) {
				QPoint p = m_textView->getByteCoordinates(selection->begin);
				m_plainTextViewScrollArea->verticalScrollBar()->setValue(p.y());
				m_plainTextViewScrollArea->horizontalScrollBar()->setValue(p.x());
				m_textView->highlight(selection.value());
			} else {
				m_textView->selectNone();
			}
		} else if (tab == 1) {
			auto selection = m_textView->selection();
			if (selection.has_value()) {
				QPoint p = m_hexView->getByteCoordinates(selection->begin);
				m_hexViewScrollArea->verticalScrollBar()->setValue(p.y());
				m_hexView->highlight(selection.value());
			} else {
				m_hexView->selectNone();
			}
		}
	});

	refreshStatusBar();

	if (checkForUpdates) {
		LatestReleaseChecker *latestReleaseChecker = new LatestReleaseChecker(this);

		connect(latestReleaseChecker, &LatestReleaseChecker::failedToGetLatestRelease, [updateStatusBarLabel]() {
			updateStatusBarLabel->setText("Failed to get latest version");
		});
		connect(latestReleaseChecker, &LatestReleaseChecker::latestReleaseFound, [updateStatusBarLabel](const LatestReleaseChecker::Release &release) {
			if (release.isNewerThan(VERSION))
				updateStatusBarLabel->setText("<a href=\"example.com\">A newer version is available</a>");
			else
				updateStatusBarLabel->setText("Application is up to date");
		});
		connect(updateStatusBarLabel, &QLabel::linkActivated, [this, latestReleaseChecker]() {
			CheckForUpdatesDialog dialog(&latestReleaseChecker->latestRelease(), this);
			dialog.exec();
		});

		latestReleaseChecker->checkLatestRelease();
	}
}

void MainWindow::sendDataFromInput()
{
	QString input = m_inputField->currentText();
	if (input.isEmpty())
		return;

	m_inputField->onInputEntered();

	// Open port and send the data
	if (!m_port->isOpen())
		if (!m_port->open())
			return;
	m_port->writeFormattedData(input);
}

void MainWindow::closePort()
{
	m_port->close();
}

void MainWindow::displayData(const QByteArray &data)
{
	m_totalBytesRead += data.size();
	m_receivedData.append(data);

	{
		auto scrollBar = m_plainTextViewScrollArea->verticalScrollBar();
		int value = scrollBar->value();
		bool atBottom = (value == scrollBar->maximum());
		m_textView->insertData(data);
		if (atBottom)
			scrollBar->setValue(scrollBar->maximum());
		else
			scrollBar->setValue(value);
	}

	{
		auto scrollBar = m_hexViewScrollArea->verticalScrollBar();
		int value = scrollBar->value();
		bool atBottom = (value == scrollBar->maximum());
		m_hexView->insertData(data);
		if (atBottom)
			scrollBar->setValue(scrollBar->maximum());
		else
			scrollBar->setValue(value);
	}

	if (m_byteReceiveTimesDialog)
		m_byteReceiveTimesDialog->insertData(data);

	trimData();

	refreshStatusBar();
}

void MainWindow::toggleConnect()
{
	if (m_port->isOpen())
		closePort();
	else
		m_port->open();
}

void MainWindow::onPortOpened()
{
	for (QWidget *w : {m_portSelect, m_baudRateSelect,
		 m_dataBitsSelect, m_paritySelect, m_stopBitsSelect})
		w->setEnabled(false);

	Config c;
	if (c.rememberLastUsedPort()) {
		c.setLastUsedPort(m_port->portName());
		c.setLastUsedPortBaudRate(m_baudRateSelect->currentText());
		c.setLastUsedPortDataBits(m_dataBitsSelect->currentText());
		c.setLastUsedPortParity(m_paritySelect->currentText());
		c.setLastUsedPortStopBits(m_stopBitsSelect->currentText());
	}

	m_connectButton->setText("Disconnect");
}

void MainWindow::onPortClosed()
{
	if (m_portSelect->currentIndex() != 0)
		for (QWidget *w : {m_baudRateSelect,
			 m_dataBitsSelect, m_paritySelect, m_stopBitsSelect})
			w->setEnabled(true);

	m_portSelect->setEnabled(true);

	m_connectButton->setText(" Connect  ");

	// Close the pinout signals dialog if it's opened
	if (m_pinoutSignalsDialog) {
		m_pinoutSignalsDialog->deleteLater();
		m_pinoutSignalsDialog = nullptr;
	}
}

void MainWindow::showAsciiTable()
{
	m_asciiTableDialog->show();
	m_asciiTableDialog->activateWindow();
	m_asciiTableDialog->raise();
}

void MainWindow::showEscapeCodes()
{
	m_escapeCodesDialog->show();
	m_escapeCodesDialog->activateWindow();
	m_escapeCodesDialog->raise();
}

void MainWindow::showKeyboardShortcuts()
{
	m_keyboardShortcutsWindow->show();
	m_keyboardShortcutsWindow->activateWindow();
	m_keyboardShortcutsWindow->raise();
}

void MainWindow::sendFromFile()
{
	bool portWasOpen = m_port->isOpen();
	if (!m_port->isOpen())
		if (!m_port->open())
			return;

	QString fileName = QFileDialog::getOpenFileName(this);
	if (fileName.isEmpty())
		return;

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this, "Error", "Failed to open file: " + file.errorString());
		return;
	}

	qint64 fileSize = file.size();

	if (QMessageBox::StandardButton::Yes !=
			QMessageBox::question(this, "Send from file?",
								  "Send " + QString::number(fileSize) +
								  " bytes from file to " +
								  m_portSelect->currentText())) {
		return;
	}

	QByteArray data = file.readAll();

	SendFileDialog dialog(data.size(), this);

	connect(m_port, &SerialPort::bytesWritten, &dialog, &SendFileDialog::onBytesSent);

	m_port->writeRawData(data);
	QTime startTime = QTime::currentTime();

	if (dialog.exec()) {
		QTime endTime = QTime::currentTime();
		double secondsDelta = startTime.msecsTo(endTime) / 1000.0;
		QMessageBox::information(this, "Success", "Data successfully sent in " + QString::number(secondsDelta, 'g', 2) + " seconds");
	} else {
		if (!portWasOpen)
			closePort();
	}
}

void MainWindow::exportData()
{
	ExportDialog dialog(m_receivedData, m_textView, m_hexView, m_byteReceiveTimesDialog, this);
	dialog.exec();
}

void MainWindow::refreshPorts()
{
	QStringList ports;
	ports << "Loopback";
	for (auto portInfo : QSerialPortInfo::availablePorts())
		ports << portInfo.portName();

#if defined(Q_OS_UNIX)
	if (Config().includePtsDirectory()) {
		QDir ptsDir("/dev/pts/");
		ptsDir.setFilter(QDir::AllEntries | QDir::System);
		for (auto filename : ptsDir.entryList()) {
			bool ok;
			filename.toInt(&ok);
			if (ok)
				ports << (QString("pts/") + filename);
		}
	}
#endif

	ports.removeAll("Loopback");
	ports.sort();
	ports.insert(0, "Loopback");

	for (int i = 0; i < m_portSelect->count(); ++i) {
		if (!ports.contains(m_portSelect->itemText(i))) {
			m_portSelect->setItemData(i, QBrush(Qt::red), Qt::TextColorRole);
		} else {
			m_portSelect->setItemData(i,
									  m_portSelect->itemData(0, Qt::TextColorRole),
									  Qt::TextColorRole);
		}
	}

	QStringList selectPorts;
	for (int i = 0; i < m_portSelect->count(); ++i)
		selectPorts << m_portSelect->itemText(i);

	for (const QString &port : ports) {
		if (!selectPorts.contains(port)) {
			int position = 0;
			if (port != "Loopback") {
				position = 1;
				for (; position < selectPorts.count(); ++position)
					if (port < selectPorts[position])
						break;
			}
			selectPorts.insert(position, port);
			m_portSelect->insertItem(position, port);
		}
	}
}

void MainWindow::continuousSend()
{
	if (!m_continuousSendWindow->isVisible()) {
		m_continuousSendWindow->show();
		m_continuousSendWindow->activateWindow();
		m_continuousSendWindow->raise();
		m_continuousSendWindow->setInput(m_inputField->currentText());
		return;
	}
}

void MainWindow::sendSequence()
{
	m_sendSequenceWindow->show();
	m_sendSequenceWindow->activateWindow();
	m_sendSequenceWindow->raise();
}

void MainWindow::clearScreen()
{
	m_receivedData.clear();
	m_totalBytesRead = 0;
	m_totalBytesWritten = 0;
	m_hexView->clear();
	m_textView->clear();
	if (m_byteReceiveTimesDialog)
		m_byteReceiveTimesDialog->clear();
	refreshStatusBar();
}

void MainWindow::showPinoutSignals()
{
	if (m_pinoutSignalsDialog) {
		m_pinoutSignalsDialog->show();
		m_pinoutSignalsDialog->activateWindow();
		m_pinoutSignalsDialog->raise();
		return;
	}

	if (!m_port->isOpen())
		if (!m_port->open())
			return;

	QDialog *dialog = new QDialog(this);
	if (m_pinoutSignalsDialog)
		delete m_pinoutSignalsDialog;
	m_pinoutSignalsDialog = dialog;

	QVBoxLayout *layout = new QVBoxLayout;
	dialog->setLayout(layout);

	QCheckBox *tx = new QCheckBox;
	QCheckBox *rx = new QCheckBox;
	QCheckBox *dtr = new QCheckBox;
	QCheckBox *dcd = new QCheckBox;
	QCheckBox *dsr = new QCheckBox;
	QCheckBox *rng = new QCheckBox;
	QCheckBox *rts = new QCheckBox;
	QCheckBox *cts = new QCheckBox;
	QCheckBox *std = new QCheckBox;
	QCheckBox *srd = new QCheckBox;

	QGridLayout *in = new QGridLayout;
	layout->addLayout(in);
	int col = 0;
	for (auto label : {"TxD", "RxD", "DTR", "DCD", "DSR", "RNG", "RTS", "CTS", "STD", "SRD"})
		in->addWidget(new QLabel(label), 0, col++, Qt::AlignCenter);
	col = 0;
	for (QWidget *w : {tx, rx, dtr, dcd, dsr, rng, rts, cts, std, srd}) {
		in->addWidget(w, 1, col++, Qt::AlignCenter);
		w->setEnabled(false);
	}

	dtr->setEnabled(true);
	rts->setEnabled(true);

	QTimer *timer = new QTimer(dialog);
	timer->start(50);
	connect(timer, &QTimer::timeout, [this, tx, rx, dtr, dcd, dsr, rng, rts, cts, std, srd](){
		QSerialPort::PinoutSignals s = m_port->pinoutSignals();
		tx->setChecked(s & QSerialPort::TransmittedDataSignal);
		rx->setChecked(s & QSerialPort::ReceivedDataSignal);
		dtr->setChecked(s & QSerialPort::DataTerminalReadySignal);
		dcd->setChecked(s & QSerialPort::DataCarrierDetectSignal);
		dsr->setChecked(s & QSerialPort::DataSetReadySignal);
		rng->setChecked(s & QSerialPort::RingIndicatorSignal);
		rts->setChecked(s & QSerialPort::RequestToSendSignal);
		cts->setChecked(s & QSerialPort::ClearToSendSignal);
		std->setChecked(s & QSerialPort::SecondaryTransmittedDataSignal);
		srd->setChecked(s & QSerialPort::SecondaryReceivedDataSignal);
	});

	connect(dtr, &QCheckBox::clicked, [this, dialog, dtr](bool state){
		if (!m_port->setDataTerminalReady(bool(state)))
			QMessageBox::critical(dialog, "Error",
								  "Failed to " + (state ? QString("set") : QString("clear")) +
								  " DTR: " + m_port->errorString());
		dtr->setChecked(m_port->isDataTerminalReady());
	});

	connect(rts, &QCheckBox::clicked, [this, dialog, rts](bool state){
		if (!m_port->setRequestToSend(bool(state)))
			QMessageBox::critical(dialog, "Error",
								  "Failed to " + (state ? QString("set") : QString("clear")) +
								  " RTS: " + m_port->errorString());
		rts->setChecked(m_port->isRequestToSend());
	});

	dialog->show();

	connect(dialog, &QDialog::finished, [this]() {
		delete m_pinoutSignalsDialog;
		m_pinoutSignalsDialog = nullptr;
	});
}

void MainWindow::showByteReceiveTimes()
{
	if (m_byteReceiveTimesDialog) {
		m_byteReceiveTimesDialog->show();
		m_byteReceiveTimesDialog->activateWindow();
		m_byteReceiveTimesDialog->raise();
		return;
	}

	if (m_byteReceiveTimesDialog)
		delete m_byteReceiveTimesDialog;

	m_byteReceiveTimesDialog = new ByteReceiveTimesDialog(height(), this);
	m_byteReceiveTimesDialog->show();

	QPoint topRight = geometry().topRight();
	m_byteReceiveTimesDialog->move(topRight.x(), topRight.y());

	connect(m_byteReceiveTimesDialog, &QDialog::finished, [this]() {
		delete m_byteReceiveTimesDialog;
		m_byteReceiveTimesDialog = nullptr;
	});
}

void MainWindow::showCheckForUpdates()
{
	CheckForUpdatesDialog dialog(nullptr, this);
	dialog.exec();
}

void MainWindow::showChangelog()
{
	ChangelogDialog dialog(this);
	dialog.exec();
}

void MainWindow::showLicense()
{
	QFile file(":/LICENSE.txt");
	bool fileOpened = file.open(QIODevice::ReadOnly);
	Q_ASSERT(fileOpened);
	QString text = file.readAll();
	QMessageBox::about(this, "MIT License", text);
}

void MainWindow::showAboutPage()
{
	QMessageBox::about(this, APPLICATION_NAME,
	tr("<p><b>" APPLICATION_NAME "</b> is a free, open-source serial port access "
	   "application for Linux and Windows written in C++ with Qt5.</p>"
	   "<p>You are currently using <b>version " VERSION "</b>.</p>"
	   "<p>It is licensed under the <a href=\"https://opensource.org/licenses/MIT\">MIT License</a>.</p>"
	   "<p>The source code can be found <a href=\"" SOURCE_CODE_URL "\">here</a>.<br/>"
	   "All releases can be found <a href=\"" RELEASES_URL "\">here</a>.</p>"));

}

void MainWindow::showAboutQtPage()
{
	QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::showSettings()
{
	m_settingsPage->show();
	m_settingsPage->activateWindow();
	m_settingsPage->raise();
}

void MainWindow::refreshStatusBar()
{
	m_statusBarLabel->setText("Bytes read: " + QString::number(m_totalBytesRead) +
							 ", Bytes written: " + QString::number(m_totalBytesWritten));
}

void MainWindow::trimData()
{
	quint64 limit = quint64(Config().readBufferLimitKiB()) * 1024;

	if (quint64(m_receivedData.size()) >= limit + limit / 5) {
		int delta = int(limit / 5);
		m_receivedData.remove(0, delta);

		m_textView->setData(m_receivedData);
		m_hexView->setData(m_receivedData);
	}

	if (m_byteReceiveTimesDialog) {
		if (quint64(m_byteReceiveTimesDialog->bytesCount()) >= limit + limit / 5) {
			int delta = int(limit / 5);
			m_byteReceiveTimesDialog->removeFromBegining(delta);
		}
	}
}


void MainWindow::resizeEvent(QResizeEvent *)
{
	m_hexViewBytesPerLine->move(m_tabs->width() - m_hexViewBytesPerLine->width(), 0);
}
