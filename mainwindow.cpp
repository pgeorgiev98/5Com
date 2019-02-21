#include "mainwindow.h"
#include "hexview.h"
#include "plaintextview.h"
#include "asciitable.h"
#include "bytereceivetimesdialog.h"
#include "sendfiledialog.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>
#include <QScrollBar>
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

#define APPLICATION_NAME "5Com"
#define VERSION "0.1.0"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, m_totalBytesWritten(0)
	, m_statusBarLabel(new QLabel)
	, m_portSelect(new QComboBox)
	, m_baudRateSelect(new QComboBox)
	, m_dataBitsSelect(new QComboBox)
	, m_paritySelect(new QComboBox)
	, m_stopBitsSelect(new QComboBox)
	, m_connectButton(new QPushButton)
	, m_inputField(new QLineEdit)
	, m_textView(new PlainTextView)
	, m_hexView(new HexView)
	, m_port(new QSerialPort(this))
	, m_opened(false)
	, m_usingLoopback(false)
	, m_continuousSendDialog(nullptr)
	, m_pinoutSignalsDialog(nullptr)
	, m_byteReceiveTimesDialog(nullptr)
{
	setMinimumSize(640, 480);
	setWindowIcon(QIcon(":/icon.ico"));

	statusBar()->addPermanentWidget(m_statusBarLabel);

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
	});


	QWidget *central = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;
	central->setLayout(layout);
	setCentralWidget(central);

	QTabWidget *tabs = new QTabWidget;
	tabs->addTab(m_textView, "Plain Text View");
	tabs->addTab(m_hexView, "Hex View");

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
		inputLayout->addWidget(m_inputField);
		inputLayout->addWidget(sendButton);

		layout->addLayout(inputLayout);

		connect(sendButton, &QPushButton::clicked, this, &MainWindow::sendDataFromMainInput);
		connect(m_inputField, &QLineEdit::returnPressed, this, &MainWindow::sendDataFromMainInput);
	}

	layout->addWidget(tabs);

	connect(m_connectButton, &QPushButton::clicked, this, &MainWindow::toggleConnect);

	QAction *writeFile = new QAction("Write &file");
	QAction *exportAction = new QAction("&Export");
	QAction *continuousSendAction = new QAction("&Continuous send");
	QAction *pinoutSignalsAction = new QAction("&Pinout signals");
	QAction *byteReceiveTimesAction = new QAction("&Byte receive times");
	QAction *clearScreenAction = new QAction("C&lear screen");
	QAction *asciiAction = new QAction("ASCII &table");
	QAction *escapeCodesAction = new QAction("&Escape codes");
	QAction *licenseAction = new QAction("&License");
	QAction *aboutQtAction = new QAction("About &Qt");
	QAction *aboutAction = new QAction("&About");

	auto fileMenu = menuBar()->addMenu("&File");
	fileMenu->addAction(writeFile);
	fileMenu->addAction(exportAction);

	auto toolsMenu = menuBar()->addMenu("&Tools");
	toolsMenu->addAction(continuousSendAction);
	toolsMenu->addAction(pinoutSignalsAction);
	toolsMenu->addAction(byteReceiveTimesAction);
	toolsMenu->addAction(clearScreenAction);

	auto helpMenu = menuBar()->addMenu("&Help");
	helpMenu->addAction(asciiAction);
	helpMenu->addAction(escapeCodesAction);
	helpMenu->addAction(licenseAction);
	helpMenu->addAction(aboutQtAction);
	helpMenu->addAction(aboutAction);

	connect(writeFile, &QAction::triggered, this, &MainWindow::sendFromFile);
	connect(exportAction, &QAction::triggered, this, &MainWindow::exportData);
	connect(continuousSendAction, &QAction::triggered, this, &MainWindow::continuousSend);
	connect(pinoutSignalsAction, &QAction::triggered, this, &MainWindow::showPinoutSignals);
	connect(byteReceiveTimesAction, &QAction::triggered, this, &MainWindow::showByteReceiveTimes);
	connect(clearScreenAction, &QAction::triggered, this, &MainWindow::clearScreen);
	connect(asciiAction, &QAction::triggered, this, &MainWindow::showAsciiTable);
	connect(escapeCodesAction, &QAction::triggered, this, &MainWindow::showEscapeCodes);
	connect(licenseAction, &QAction::triggered, this, &MainWindow::showLicense);
	connect(aboutQtAction, &QAction::triggered, this, &MainWindow::showAboutQtPage);
	connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutPage);

	refreshPorts();
	if (m_portSelect->count() > 1)
		m_portSelect->setCurrentIndex(1);
	onPortSelectChanged();

	QTimer *refreshTimer = new QTimer(this);
	refreshTimer->start(2000);
	connect(refreshTimer, &QTimer::timeout, this, &MainWindow::refreshPorts);

	connect(m_port, &QSerialPort::readyRead, this, &MainWindow::readFromPort);
	connect(m_portSelect, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::onPortSelectChanged);

	connect(m_port, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error), [this](QSerialPort::SerialPortError error) {
		if (error != QSerialPort::SerialPortError::NoError && m_port->isOpen()) {
			QMessageBox::critical(this, APPLICATION_NAME, "Serial port error: " + m_port->errorString());
			closePort();
		}
	});

	refreshStatusBar();

	connect(this, &MainWindow::bytesWritten, [this](qint64 bytes) {
		m_totalBytesWritten += bytes;
		refreshStatusBar();
	});
}

MainWindow::~MainWindow()
{

}

void MainWindow::sendRawData(const QByteArray &data)
{
	if (!m_opened)
		if (!openPort())
			return;

	if (m_usingLoopback) {
		displayData(data);
		emit bytesWritten(data.size());
	} else {
		m_port->write(data);
	}
}

void MainWindow::sendData(const QString &input)
{
	if (!m_opened)
		if (!openPort())
			return;
	QByteArray output;
	for (int i = 0; i < input.size(); ++i) {
		QChar c = input[i];
		char newChar = c.toLatin1();
		if (c == '\\' && i + 1 < input.size()) {
			if (input[i + 1] == '\\') {
				newChar = '\\';
				++i;
			} else if (input[i + 1] == 'n') {
				newChar = '\n';
				++i;
			} else if (input[i + 1] == 'r') {
				newChar = '\r';
				++i;
			} else if (input[i + 1] == 't') {
				newChar = '\t';
				++i;
			} else if (input[i + 1] == 'x' && i + 3 < input.size() &&
					 ((input[i + 2] >= '0' && input[i + 2] <= '9') ||
					  (input[i + 2].toLower() >= 'a' && input[i + 2].toLower() <= 'f')) &&
					 ((input[i + 3] >= '0' && input[i + 3] <= '9') ||
					  (input[i + 3].toLower() >= 'a' && input[i + 3].toLower() <= 'f'))) {
				newChar = char(QString(input[i + 2]).toInt(nullptr, 16) * 16 +
						QString(input[i + 3]).toInt(nullptr, 16));
						i += 3;
			}
		}
		output.append(newChar);
	}
	sendRawData(output);
}

bool MainWindow::openPort()
{
	if (m_portSelect->currentIndex() == 0) {
		m_usingLoopback = true;
	} else {
		m_usingLoopback = false;
		m_port->setPortName(m_portSelect->currentText());
		m_port->setBaudRate(m_baudRateSelect->currentText().toInt());
		m_port->setDataBits(QSerialPort::DataBits(m_dataBitsSelect->currentText().toInt()));
		static const QSerialPort::Parity parity[5] = {QSerialPort::EvenParity,
													  QSerialPort::MarkParity,
													  QSerialPort::OddParity,
													  QSerialPort::SpaceParity,
													  QSerialPort::NoParity};
		m_port->setParity(parity[m_paritySelect->currentIndex()]);
		static const QSerialPort::StopBits stopBits[3] = {QSerialPort::OneStop,
														  QSerialPort::OneAndHalfStop,
														  QSerialPort::TwoStop};
		m_port->setStopBits(stopBits[m_stopBitsSelect->currentIndex()]);

		if (!m_port->open(QIODevice::ReadWrite)) {
			QMessageBox::critical(this, "Error", "Failed to open port: " + m_port->errorString());
			return false;
		}
		connect(m_port, &QIODevice::bytesWritten, this, &MainWindow::bytesWritten);
	}

	for (QWidget *w : {m_portSelect, m_baudRateSelect,
		 m_dataBitsSelect, m_paritySelect, m_stopBitsSelect})
		w->setEnabled(false);

	m_connectButton->setText("Disconnect");

	m_opened = true;

	return true;
}

void MainWindow::closePort()
{
	m_opened = false;

	if (!m_usingLoopback) {
		m_port->close();
		for (QWidget *w : {m_baudRateSelect,
			 m_dataBitsSelect, m_paritySelect, m_stopBitsSelect})
			w->setEnabled(true);
		disconnect(m_port, &QIODevice::bytesWritten, this, &MainWindow::bytesWritten);
	}

	m_portSelect->setEnabled(true);

	m_connectButton->setText(" Connect  ");
}

void MainWindow::displayData(const QByteArray &data)
{
	m_receivedData.append(data);

	m_hexView->insertData(data);

	m_textView->insertData(data);

	if (m_byteReceiveTimesDialog)
		m_byteReceiveTimesDialog->display(data);

	refreshStatusBar();
}

void MainWindow::toggleConnect()
{
	if (m_opened)
		closePort();
	else
		openPort();
}

void MainWindow::showAsciiTable()
{
	AsciiTable asciiTable;
	asciiTable.exec();
}

void MainWindow::showEscapeCodes()
{
	QString text = "\
Escape codes:\n\n\
\\\\   - Backslash (\\)\n\
\\n   - Linefeed (LF)\n\
\\r   - Carriage return (CR)\n\
\\t   - Horizontal Tab\n\
\\xHH - Byte with a hex value of HH\
";
	QDialog dialog(this);
	QHBoxLayout *layout = new QHBoxLayout;
	QLabel *label = new QLabel;
	label->setText(text);
	label->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	label->setWordWrap(true);
	label->setTextFormat(Qt::TextFormat::PlainText);
	layout->addWidget(label);
	dialog.setLayout(layout);
	dialog.exec();
}

void MainWindow::sendFromFile()
{
	if (!m_opened)
		if (!openPort())
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

	connect(this, &MainWindow::bytesWritten, &dialog, &SendFileDialog::onBytesSent);

	sendRawData(data);
	QTime startTime = QTime::currentTime();

	if (dialog.exec()) {
		QTime endTime = QTime::currentTime();
		double secondsDelta = startTime.msecsTo(endTime) / 1000.0;
		QMessageBox::information(this, "Success", "Data successfully sent in " + QString::number(secondsDelta, 'g', 2) + " seconds");
	} else {
		closePort();
		openPort();
	}
}

void MainWindow::exportData()
{
	QFileDialog dialog(this, "Export", "", "TEXT (*.txt);;HEX (*.hex);;RAW (*)");
	dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
	if (!dialog.exec() || dialog.selectedFiles().isEmpty())
		return;
	QString path = dialog.selectedFiles()[0];
	if (!path.endsWith(".hex") && !path.endsWith(".txt")) {
		if (dialog.selectedNameFilter() == "TEXT (*.txt)")
			path.append(".txt");
		else if (dialog.selectedNameFilter() == "HEX (*.hex)")
			path.append(".hex");
	}
	if (dialog.selectedNameFilter() == "RAW (*)")
		exportRaw(path);
	else if (dialog.selectedNameFilter() == "TEXT (*.txt)")
		exportAsText(path);
	else if (dialog.selectedNameFilter() == "HEX (*.hex)")
		exportAsHex(path);
	else
		Q_ASSERT(false);
}

void MainWindow::exportRaw(const QString &path)
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, "Error", "Failed to open file: " + file.errorString());
		return;
	}
	file.write(m_receivedData);
}

void MainWindow::exportAsText(const QString &path)
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, "Error", "Failed to open file: " + file.errorString());
		return;
	}
	if (file.write(m_textView->toPlainText().toLatin1()) < 0) {
		QMessageBox::critical(this, "Error", "Failed to write to file: " + file.errorString());
		return;
	}
}

void MainWindow::exportAsHex(const QString &path)
{
	QFile file(path);
	if (!file.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, "Error", "Failed to open file: " + file.errorString());
		return;
	}
	if (file.write(m_hexView->toPlainText().toLatin1()) < 0) {
		QMessageBox::critical(this, "Error", "Failed to write to file: " + file.errorString());
		return;
	}
}

void MainWindow::readFromPort()
{
	displayData(m_port->readAll());
}

void MainWindow::refreshPorts()
{
	QStringList ports;
	ports << "Loopback";
	for (auto portInfo : QSerialPortInfo::availablePorts())
		ports << portInfo.portName();

#if !defined(QT_NO_DEBUG) && defined(Q_OS_UNIX)
	QDir ptsDir("/dev/pts/");
	ptsDir.setFilter(QDir::AllEntries | QDir::System);
	for (auto filename : ptsDir.entryList()) {
		bool ok;
		filename.toInt(&ok);
		if (ok)
			ports << (QString("pts/") + filename);
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

static int packetsSent = 0;
void MainWindow::continuousSend()
{
	if (m_continuousSendDialog) {
		m_continuousSendDialog->show();
		m_continuousSendDialog->activateWindow();
		m_continuousSendDialog->raise();
		return;
	}

	if (!m_opened)
		if (!openPort())
			return;

	QDialog *dialog = new QDialog(this);
	if (m_continuousSendDialog)
		delete m_continuousSendDialog;
	m_continuousSendDialog = dialog;

	QStackedLayout *layout = new QStackedLayout;
	dialog->setLayout(layout);

	QWidget *setupWidget = new QWidget;
	layout->addWidget(setupWidget);
	QGridLayout *setupLayout = new QGridLayout;
	setupWidget->setLayout(setupLayout);

	QLineEdit *input = new QLineEdit;
	input->setText(m_inputField->text());
	QSpinBox *interval = new QSpinBox;
	interval->setRange(0, 1000000000);
	interval->setValue(200);
	interval->setSuffix("ms");
	QCheckBox *sendIndefinitely = new QCheckBox("Send indefinitely");
	sendIndefinitely->setChecked(true);
	QSpinBox *packetCount = new QSpinBox;
	packetCount->setRange(1, 1000000000);
	packetCount->setEnabled(false);
	QPushButton *sendButton = new QPushButton("Send");

	int row = 0;
	setupLayout->addWidget(input, row++, 0, 1, 2);

	setupLayout->addWidget(new QLabel("Send interval: "), row, 0);
	setupLayout->addWidget(interval, row++, 1);

	setupLayout->addWidget(sendIndefinitely, row++, 0, 1, 2);

	setupLayout->addWidget(new QLabel("Number of messages to send: "), row, 0);
	setupLayout->addWidget(packetCount, row++, 1);

	setupLayout->addWidget(sendButton, row, 0, 1, 2);

	QWidget *sendingWidget = new QWidget;
	layout->addWidget(sendingWidget);
	QVBoxLayout *sendingLayout = new QVBoxLayout;
	sendingWidget->setLayout(sendingLayout);

	QLabel *text = new QLabel();
	QPushButton *cancelButton = new QPushButton("Cancel");
	sendingLayout->addWidget(text, 0, Qt::AlignCenter);
	sendingLayout->addWidget(cancelButton);


	QTimer *sendTimer = new QTimer(dialog);

	auto cancel = [layout, sendTimer]() {
		sendTimer->stop();
		layout->setCurrentIndex(0);
	};

	auto sendPacket = [&packetsSent, this, input, sendIndefinitely, packetCount, text, cancel]() {
		sendData(input->text());
		++packetsSent;
		text->setText(QString::number(packetsSent) + " messages sent");
		if (!sendIndefinitely->isChecked() && packetsSent >= packetCount->value())
			cancel();
	};

	connect(sendButton, &QPushButton::clicked, [sendTimer, &packetsSent, input, layout, interval, text, sendPacket]() {
		if (input->text().isEmpty())
			return;
		packetsSent = 0;
		text->setText("0 messages sent");
		layout->setCurrentIndex(1);
		sendPacket();
		sendTimer->start(interval->value());
	});

	connect(cancelButton, &QPushButton::clicked, cancel);
	connect(sendTimer, &QTimer::timeout, sendPacket);
	connect(sendIndefinitely, &QCheckBox::stateChanged, packetCount, &QSpinBox::setDisabled);

	dialog->show();

	connect(dialog, &QDialog::finished, [this]() {
		delete m_continuousSendDialog;
		m_continuousSendDialog = nullptr;
	});
}

void MainWindow::onPortSelectChanged()
{
	bool enabled = m_portSelect->currentIndex() != 0;
	for (QWidget *w : {m_baudRateSelect, m_dataBitsSelect, m_paritySelect, m_stopBitsSelect})
		w->setEnabled(enabled);
}

void MainWindow::clearScreen()
{
	m_receivedData.clear();
	m_totalBytesWritten = 0;
	m_hexView->clear();
	m_textView->clear();
	if (m_byteReceiveTimesDialog)
		m_byteReceiveTimesDialog->clear();
	refreshStatusBar();
}

void MainWindow::sendDataFromMainInput()
{
	sendData(m_inputField->text());
}

void MainWindow::showPinoutSignals()
{
	if (m_pinoutSignalsDialog) {
		m_pinoutSignalsDialog->show();
		m_pinoutSignalsDialog->activateWindow();
		m_pinoutSignalsDialog->raise();
		return;
	}

	if (!m_opened)
		if (!openPort())
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

void MainWindow::showLicense()
{
	QFile file(":/LICENSE.txt");
	Q_ASSERT(file.open(QIODevice::ReadOnly));
	QString text = file.readAll();
	QMessageBox::about(this, "MIT License", text);
}

void MainWindow::showAboutPage()
{
	QMessageBox::about(this, APPLICATION_NAME,
	tr("<p><b>" APPLICATION_NAME "</b> is a free serial port access "
	   "application for Linux and Windows written in C++ with Qt5.</p>"
	   "<p>You are currently using <b>version " VERSION "</b></p>"
	   "<p>It is licensed under the <a href=\"https://opensource.org/licenses/MIT\">MIT License</a></p>"));

}

void MainWindow::showAboutQtPage()
{
	QMessageBox::aboutQt(this, "About Qt");
}

void MainWindow::refreshStatusBar()
{
	m_statusBarLabel->setText("Bytes read: " + QString::number(m_receivedData.size()) +
							 ", Bytes written: " + QString::number(m_totalBytesWritten));
}
