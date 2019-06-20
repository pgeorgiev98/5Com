#include "settingspage.h"
#include "line.h"
#include "config.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

QHBoxLayout *labeledWidget(const QString &label, QWidget *widget)
{
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(new QLabel(label));
	layout->addWidget(widget);
	return layout;
}

SettingsPage::SettingsPage(QWidget *parent)
	: QDialog(parent)
#if defined(Q_OS_UNIX)
	, m_includePtsDirectory(new QCheckBox("Include files from /dev/pts/ as serial ports"))
#else
	, m_includePtsDirectory(nullptr)
#endif
	, m_checkForUpdatesOnStartup(new QCheckBox("Check for updates on startup"))
	, m_readBufferLimitKiB(new QSpinBox)
	, m_inputHistoryLength(new QSpinBox)
	, m_clearInputOnSend(new QCheckBox("Clear the input field on send"))
{
	m_readBufferLimitKiB->setSuffix("KiB");
	m_readBufferLimitKiB->setRange(1, std::numeric_limits<int>::max());
	m_inputHistoryLength->setRange(1, 10000);

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("5Com settings"), 0, Qt::AlignHCenter);
	layout->addWidget(new Line(Line::Type::Horizontal));

	if (m_includePtsDirectory)
		layout->addWidget(m_includePtsDirectory);

	layout->addWidget(m_checkForUpdatesOnStartup);

	layout->addLayout(labeledWidget("Read buffer limit: ", m_readBufferLimitKiB));
	layout->addLayout(labeledWidget("Input history length: ", m_inputHistoryLength));
	layout->addWidget(m_clearInputOnSend);

	{
		QHBoxLayout *hbox = new QHBoxLayout;
		QPushButton *cancel = new QPushButton("Cancel");
		QPushButton *ok = new QPushButton("Ok");
		QPushButton *apply = new QPushButton("Apply");
		apply->setAutoDefault(true);
		apply->setDefault(true);
		hbox->addStretch(1);
		hbox->addWidget(apply);
		hbox->addStretch(1);
		hbox->addWidget(cancel);
		hbox->addStretch(1);
		hbox->addWidget(ok);
		hbox->addStretch(1);
		layout->addLayout(hbox);

		connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
		connect(apply, &QPushButton::clicked, this, &SettingsPage::save);
		connect(ok, &QPushButton::clicked, this, &SettingsPage::save);
		connect(ok, &QPushButton::clicked, this, &SettingsPage::accept);
	}

	load();
}

void SettingsPage::load()
{
	Config c;
	if (m_includePtsDirectory)
		m_includePtsDirectory->setChecked(c.includePtsDirectory());
	m_checkForUpdatesOnStartup->setChecked(c.checkForUpdatesOnStartup());
	m_readBufferLimitKiB->setValue(c.readBufferLimitKiB());
	m_inputHistoryLength->setValue(c.inputHistoryLength());
	m_clearInputOnSend->setChecked(c.clearInputOnSend());
}

void SettingsPage::save()
{
	Config c;
	if (m_includePtsDirectory)
		c.setIncludePtsDirectory(m_includePtsDirectory->isChecked());
	c.setCheckForUpdatesOnStartup(m_checkForUpdatesOnStartup->isChecked());
	c.setReadBufferLimitKiB(m_readBufferLimitKiB->value());
	c.setInputHistoryLength(m_inputHistoryLength->value());
	c.setClearInputOnSend(m_clearInputOnSend->isChecked());
}
