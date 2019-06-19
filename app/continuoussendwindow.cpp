#include "continuoussendwindow.h"
#include "serialport.h"

#include <QTimer>

#include <QStackedLayout>
#include <QGridLayout>

#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

ContinuousSendWindow::ContinuousSendWindow(SerialPort *port, QWidget *parent)
	: QDialog(parent)
	, m_port(port)
	, m_sendTimer(new QTimer(this))
	, m_layout(new QStackedLayout)
	, m_input(new QLineEdit)
	, m_interval(new QSpinBox)
	, m_sendIndefinitely(new QCheckBox("Send indefinitely"))
	, m_packetCount(new QSpinBox)
	, m_packetsSendLabel(new QLabel)
{
	m_interval->setRange(0, 1000000000);
	m_interval->setValue(200);
	m_interval->setSuffix("ms");
	m_sendIndefinitely->setChecked(true);
	m_packetCount->setRange(1, 1000000000);
	m_packetCount->setEnabled(false);

	setLayout(m_layout);

	QWidget *setupWidget = new QWidget;
	m_layout->addWidget(setupWidget);
	QGridLayout *setupLayout = new QGridLayout;
	setupWidget->setLayout(setupLayout);

	QPushButton *sendButton = new QPushButton("Send");

	int row = 0;
	setupLayout->addWidget(m_input, row++, 0, 1, 2);

	setupLayout->addWidget(new QLabel("Send interval: "), row, 0);
	setupLayout->addWidget(m_interval, row++, 1);

	setupLayout->addWidget(m_sendIndefinitely, row++, 0, 1, 2);

	setupLayout->addWidget(new QLabel("Number of packets to send: "), row, 0);
	setupLayout->addWidget(m_packetCount, row++, 1);

	setupLayout->addWidget(sendButton, row, 0, 1, 2);

	QWidget *sendingWidget = new QWidget;
	m_layout->addWidget(sendingWidget);
	QVBoxLayout *sendingLayout = new QVBoxLayout;
	sendingWidget->setLayout(sendingLayout);

	QPushButton *cancelButton = new QPushButton("Cancel");
	sendingLayout->addWidget(m_packetsSendLabel, 0, Qt::AlignCenter);
	sendingLayout->addWidget(cancelButton);

	connect(sendButton, &QPushButton::clicked, this, &ContinuousSendWindow::startSending);
	connect(cancelButton, &QPushButton::clicked, this, &ContinuousSendWindow::stopSending);
	connect(m_sendTimer, &QTimer::timeout, this, &ContinuousSendWindow::sendPacket);
	connect(m_sendIndefinitely, &QCheckBox::stateChanged, m_packetCount, &QSpinBox::setDisabled);
	connect(this, &QDialog::finished, this, &ContinuousSendWindow::stopSending);
}

void ContinuousSendWindow::setInput(const QString &text)
{
	m_input->setText(text);
}

void ContinuousSendWindow::startSending()
{
	if (m_input->text().isEmpty())
		return;

	if (!m_port->isOpen())
		if (!m_port->open())
			return;

	m_packetsSent = 0;
	m_packetsSendLabel->setText("0 packets sent");
	m_layout->setCurrentIndex(1);

	sendPacket();
	m_sendTimer->start(m_interval->value());
}

void ContinuousSendWindow::stopSending()
{
	m_sendTimer->stop();
	m_layout->setCurrentIndex(0);
}

void ContinuousSendWindow::sendPacket()
{
	m_port->writeFormattedData(m_input->text());
	++m_packetsSent;
	m_packetsSendLabel->setText(QString::number(m_packetsSent) + " packets sent");
	if (!m_sendIndefinitely->isChecked() && m_packetsSent >= m_packetCount->value())
		stopSending();
}
