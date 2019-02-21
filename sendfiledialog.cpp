#include "sendfiledialog.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProgressBar>

SendFileDialog::SendFileDialog(qint64 bytesToSend, QWidget *parent)
	: QDialog(parent)
	, m_totalBytesSent(0)
	, m_bytesToSend(bytesToSend)
	, m_text(new QLabel)
	, m_progressBar(new QProgressBar)
{
	m_progressBar->setMinimum(0);
	m_progressBar->setMaximum(100);

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);
	QPushButton *cancel = new QPushButton("Cancel");
	layout->addWidget(m_text);
	layout->addWidget(m_progressBar);
	layout->addWidget(cancel);

	connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	onBytesSent(0);
}

void SendFileDialog::onBytesSent(qint64 bytes)
{
	m_totalBytesSent += bytes;
	m_text->setText(QString::number(m_totalBytesSent) + "/" + QString::number(m_bytesToSend) + " bytes sent");
	m_progressBar->setValue((m_totalBytesSent * 100) / m_bytesToSend);
	if (m_totalBytesSent >= m_bytesToSend)
		accept();
}
