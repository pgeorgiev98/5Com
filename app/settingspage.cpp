#include "settingspage.h"
#include "line.h"
#include "config.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

SettingsPage::SettingsPage(QWidget *parent)
	: QDialog(parent)
#if defined(Q_OS_UNIX)
	, m_includePtsDirectory(new QCheckBox("Include files from /dev/pts/ as serial ports"))
#else
	, m_includePtsDirectory(nullptr)
#endif
{
	Config c;
	if (m_includePtsDirectory)
		m_includePtsDirectory->setChecked(c.includePtsDirectory());

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("5Com settings"), 0, Qt::AlignHCenter);
	layout->addWidget(new Line(Line::Type::Horizontal));

	if (m_includePtsDirectory)
		layout->addWidget(m_includePtsDirectory);

	{
		QHBoxLayout *hbox = new QHBoxLayout;
		QPushButton *cancel = new QPushButton("Cancel");
		QPushButton *apply = new QPushButton("Apply");
		hbox->addStretch(1);
		hbox->addWidget(cancel);
		hbox->addStretch(1);
		hbox->addWidget(apply);
		hbox->addStretch(1);
		layout->addLayout(hbox);

		connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
		connect(apply, &QPushButton::clicked, this, &SettingsPage::save);
	}
}

void SettingsPage::load()
{
	Config c;
	if (m_includePtsDirectory)
		m_includePtsDirectory->setChecked(c.includePtsDirectory());
}

void SettingsPage::save()
{
	Config c;
	if (m_includePtsDirectory)
		c.setIncludePtsDirectory(m_includePtsDirectory->isChecked());

	accept();
}
