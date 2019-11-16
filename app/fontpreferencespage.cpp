#include "fontpreferencespage.h"
#include "line.h"
#include "common.h"
#include "config.h"

#include <QVBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QFontDialog>
#include <QMessageBox>

static QHBoxLayout *labeledWidget(const QString &label, QWidget *widget)
{
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(new QLabel(label));
	layout->addWidget(widget);
	return layout;
}

FontPreferencesPage::FontPreferencesPage(QWidget *parent)
	: QDialog(parent)
	, m_useBuiltInFixedFont(new QRadioButton("Use built in fixed font (DejaVu Sans Mono)"))
	, m_useSystemFixedFont(new QRadioButton("Use the system fixed font"))
	, m_useOtherFixedFont(new QRadioButton("Specify fixed font to use"))
	, m_fixedFontName(new QLineEdit)
	, m_fixedFontSize(new QSpinBox)
	, m_fixedFontInputWidget(new QWidget)
{
	m_fixedFontName->setPlaceholderText("Not specified");
	m_fixedFontName->setReadOnly(true);
	m_fixedFontSize->setRange(1, 200);

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("Font Preferences"), 0, Qt::AlignHCenter);
	layout->addWidget(new Line(Line::Type::Horizontal));

	layout->addWidget(m_useBuiltInFixedFont);
	layout->addWidget(m_useSystemFixedFont);
	layout->addWidget(m_useOtherFixedFont);

	QPushButton *fontSelectButton = new QPushButton("Select");
	QHBoxLayout *hbox = new QHBoxLayout;
	m_fixedFontInputWidget->setLayout(hbox);
	hbox->addWidget(new QLabel("Fixed font: "));
	hbox->addWidget(m_fixedFontName);
	hbox->addWidget(fontSelectButton);

	layout->addWidget(m_fixedFontInputWidget);
	layout->addLayout(labeledWidget("Font size: ", m_fixedFontSize));

	connect(fontSelectButton, &QPushButton::clicked, [this]() {
		bool ok;
		QFont font = QFontDialog::getFont(&ok, getFixedFont(), this, "Select fixed font",
										  QFontDialog::MonospacedFonts);
		if (ok) {
			m_fixedFontName->setText(font.family());
			m_fixedFontSize->setValue(font.pointSize());
		}
	});

	QPushButton *restoreDefaultsButton = new QPushButton("Restore defaults");
	layout->addWidget(restoreDefaultsButton, 0, Qt::AlignRight);
	connect(restoreDefaultsButton, &QPushButton::clicked, this, &FontPreferencesPage::restoreDefaults);
	layout->addSpacing(8);

	{
		QHBoxLayout *hbox = new QHBoxLayout;
		QPushButton *cancel = new QPushButton("Cancel");
		QPushButton *ok = new QPushButton("Ok");
		QPushButton *apply = new QPushButton("Apply");
		apply->setAutoDefault(true);
		apply->setDefault(true);
		hbox->addStretch(1);
		hbox->addWidget(ok);
		hbox->addStretch(1);
		hbox->addWidget(cancel);
		hbox->addStretch(1);
		hbox->addWidget(apply);
		hbox->addStretch(1);
		layout->addLayout(hbox);

		connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
		connect(apply, &QPushButton::clicked, this, &FontPreferencesPage::save);
		connect(ok, &QPushButton::clicked, [this]() {
			if (save())
				accept();
		});
	}


	connect(m_useOtherFixedFont, &QRadioButton::toggled, [this](bool checked) {
		m_fixedFontInputWidget->setEnabled(checked);
		if (!checked)
			m_fixedFontName->setText(QString());
	});

	load();
}

void FontPreferencesPage::load()
{
	Config c;
	Config::Font &f = c.font;
	m_useBuiltInFixedFont->setChecked(f.useBuildInFixedFont());
	m_useSystemFixedFont->setChecked(f.useSystemFixedFont());
	m_useOtherFixedFont->setChecked(!m_useBuiltInFixedFont->isChecked() && !m_useSystemFixedFont->isChecked());
	if (m_useBuiltInFixedFont->isChecked() || m_useSystemFixedFont->isChecked()) {
		m_fixedFontName->setText(QString());
		m_fixedFontInputWidget->setEnabled(false);
	} else {
		m_fixedFontName->setText(f.fixedFontName());
		m_fixedFontInputWidget->setEnabled(true);
	}
	m_fixedFontSize->setValue(f.fixedFontSize());

	emit preferencesChanged();
}

bool FontPreferencesPage::save()
{
	if (m_useOtherFixedFont->isChecked() && m_fixedFontName->text().isEmpty()) {
		QMessageBox::information(this, "Font Preferences", "Please select a fixed font");
		return false;
	}

	Config c;
	Config::Font &f = c.font;
	f.setUseBuildInFixedFont(m_useBuiltInFixedFont->isChecked());
	f.setUseSystemFixedFont(m_useSystemFixedFont->isChecked());
	f.setFixedFontName(m_fixedFontName->text());
	f.setFixedFontSize(m_fixedFontSize->value());

	emit preferencesChanged();

	return true;
}

void FontPreferencesPage::restoreDefaults()
{
	int b = QMessageBox::question(this, "Restore defaults",
								  "Are you sure you want to restore the default font preferences?");
	if (b == QMessageBox::Yes) {
		Config().font.clear();
		load();
	}
}
