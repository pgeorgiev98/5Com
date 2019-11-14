#include "preferencespage.h"
#include "line.h"
#include "config.h"
#include "common.h"

#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QFontDialog>
#include <QMessageBox>

QHBoxLayout *labeledWidget(const QString &label, QWidget *widget)
{
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(new QLabel(label));
	layout->addWidget(widget);
	return layout;
}

PreferencesPage::PreferencesPage(QWidget *parent)
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
	, m_rememberLastUsedPort(new QCheckBox("Remember last used port and its settings"))
	, m_rememberInputHistory(new QCheckBox("Remember input history"))
	, m_hexViewBytesPerLine(new QSpinBox)
	, m_saveMainWindowSize(new QCheckBox("Remember the main window size"))
	, m_mainWindowWidth(new QSpinBox)
	, m_mainWindowHeight(new QSpinBox)
	, m_useBuiltInFixedFont(new QRadioButton("Use built in fixed font (DejaVu Sans Mono)"))
	, m_useSystemFixedFont(new QRadioButton("Use the system fixed font"))
	, m_useOtherFixedFont(new QRadioButton("Specify fixed font to use"))
	, m_fixedFontName(new QLineEdit)
	, m_fixedFontSize(new QSpinBox)
	, m_fixedFontInputWidget(new QWidget)
{
	m_readBufferLimitKiB->setSuffix("KiB");
	m_readBufferLimitKiB->setRange(1, std::numeric_limits<int>::max());
	m_inputHistoryLength->setRange(1, 10000);
	m_hexViewBytesPerLine->setRange(1, 128);
	m_mainWindowWidth->setRange(0, 20000);
	m_mainWindowWidth->setButtonSymbols(QSpinBox::ButtonSymbols::NoButtons);
	m_mainWindowHeight->setRange(0, 20000);
	m_mainWindowHeight->setButtonSymbols(QSpinBox::ButtonSymbols::NoButtons);
	m_fixedFontName->setPlaceholderText("Not specified");
	m_fixedFontName->setReadOnly(true);
	m_fixedFontSize->setRange(1, 200);

	QWidget *mainWindowSizeWidget = new QWidget;
	{
		QHBoxLayout *layout = new QHBoxLayout;
		mainWindowSizeWidget->setLayout(layout);
		layout->addWidget(new QLabel("Main window size: "));
		layout->addWidget(m_mainWindowWidth);
		layout->addWidget(new QLabel("x"));
		layout->addWidget(m_mainWindowHeight);
		layout->addStretch(1);
	}

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("5Com Preferences"), 0, Qt::AlignHCenter);
	layout->addWidget(new Line(Line::Type::Horizontal));

	if (m_includePtsDirectory)
		layout->addWidget(m_includePtsDirectory);

	layout->addWidget(m_checkForUpdatesOnStartup);

	layout->addLayout(labeledWidget("Read buffer limit: ", m_readBufferLimitKiB));
	layout->addLayout(labeledWidget("Input history length: ", m_inputHistoryLength));
	layout->addWidget(m_clearInputOnSend);
	layout->addWidget(m_rememberLastUsedPort);
	layout->addWidget(m_rememberInputHistory);
	layout->addLayout(labeledWidget("Number of bytes per line in HexView: ", m_hexViewBytesPerLine));
	layout->addWidget(m_saveMainWindowSize);
	layout->addWidget(mainWindowSizeWidget);
	layout->addWidget(m_useBuiltInFixedFont);

	{
		QGroupBox *fontPreferences = new QGroupBox("Fixed font");
		QVBoxLayout *vbox = new QVBoxLayout;
		fontPreferences->setLayout(vbox);
		vbox->addWidget(m_useBuiltInFixedFont);
		vbox->addWidget(m_useSystemFixedFont);
		vbox->addWidget(m_useOtherFixedFont);

		QPushButton *fontSelectButton = new QPushButton("Select");
		QHBoxLayout *hbox = new QHBoxLayout;
		m_fixedFontInputWidget->setLayout(hbox);
		hbox->addWidget(new QLabel("Fixed font: "));
		hbox->addWidget(m_fixedFontName);
		hbox->addWidget(fontSelectButton);

		vbox->addWidget(m_fixedFontInputWidget);
		vbox->addLayout(labeledWidget("Font size: ", m_fixedFontSize));

		layout->addWidget(fontPreferences);

		connect(fontSelectButton, &QPushButton::clicked, [this]() {
			bool ok;
			QFont font = QFontDialog::getFont(&ok, getFixedFont(), this, "Select fixed font",
											  QFontDialog::MonospacedFonts);
			if (ok) {
				m_fixedFontName->setText(font.family());
				m_fixedFontSize->setValue(font.pointSize());
			}
		});
	}

	QPushButton *restoreDefaultsButton = new QPushButton("Restore defaults");
	layout->addWidget(restoreDefaultsButton, 0, Qt::AlignRight);
	connect(restoreDefaultsButton, &QPushButton::clicked, this, &PreferencesPage::restoreDefaults);
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
		connect(apply, &QPushButton::clicked, this, &PreferencesPage::save);
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

	connect(m_saveMainWindowSize, &QCheckBox::stateChanged, mainWindowSizeWidget, &QWidget::setDisabled);
	mainWindowSizeWidget->setDisabled(m_saveMainWindowSize->isChecked());

	load();
}

void PreferencesPage::load()
{
	Config c;
	if (m_includePtsDirectory)
		m_includePtsDirectory->setChecked(c.includePtsDirectory());
	m_checkForUpdatesOnStartup->setChecked(c.checkForUpdatesOnStartup());
	m_readBufferLimitKiB->setValue(c.readBufferLimitKiB());
	m_inputHistoryLength->setValue(c.inputHistoryLength());
	m_clearInputOnSend->setChecked(c.clearInputOnSend());
	m_rememberLastUsedPort->setChecked(c.rememberLastUsedPort());
	m_rememberInputHistory->setChecked(c.saveInputHistory());
	m_hexViewBytesPerLine->setValue(c.hexViewBytesPerLine());
	m_saveMainWindowSize->setChecked(c.saveMainWindowSize());
	m_mainWindowWidth->setValue(c.mainWindowSize().width());
	m_mainWindowHeight->setValue(c.mainWindowSize().height());
	m_useBuiltInFixedFont->setChecked(c.useBuildInFixedFont());
	m_useSystemFixedFont->setChecked(c.useSystemFixedFont());
	m_useOtherFixedFont->setChecked(!m_useBuiltInFixedFont->isChecked() && !m_useSystemFixedFont->isChecked());
	if (m_useBuiltInFixedFont->isChecked() || m_useSystemFixedFont->isChecked()) {
		m_fixedFontName->setText(QString());
		m_fixedFontInputWidget->setEnabled(false);
	} else {
		m_fixedFontName->setText(c.fixedFontName());
		m_fixedFontInputWidget->setEnabled(true);
	}
	m_fixedFontSize->setValue(c.fixedFontSize());

	emit preferencesChanged();
}

bool PreferencesPage::save()
{
	if (m_useOtherFixedFont->isChecked() && m_fixedFontName->text().isEmpty()) {
		QMessageBox::information(this, "Preferences", "Please select a fixed font");
		return false;
	}

	Config c;
	if (m_includePtsDirectory)
		c.setIncludePtsDirectory(m_includePtsDirectory->isChecked());
	c.setCheckForUpdatesOnStartup(m_checkForUpdatesOnStartup->isChecked());
	c.setReadBufferLimitKiB(m_readBufferLimitKiB->value());
	c.setInputHistoryLength(m_inputHistoryLength->value());
	c.setClearInputOnSend(m_clearInputOnSend->isChecked());
	c.setRememberLastUsedPort(m_rememberLastUsedPort->isChecked());
	c.setSaveInputHistory(m_rememberInputHistory->isChecked());
	c.setHexViewBytesPerLine(m_hexViewBytesPerLine->value());
	c.setSaveMainWindowSize(m_saveMainWindowSize->isChecked());
	c.setMainWindowSize(QSize(m_mainWindowWidth->value(), m_mainWindowHeight->value()));
	c.setUseBuildInFixedFont(m_useBuiltInFixedFont->isChecked());
	c.setUseSystemFixedFont(m_useSystemFixedFont->isChecked());
	c.setFixedFontName(m_fixedFontName->text());
	c.setFixedFontSize(m_fixedFontSize->value());

	emit preferencesChanged();

	return true;
}

void PreferencesPage::restoreDefaults()
{
	int b = QMessageBox::question(this, "Restore defaults",
								  "Are you sure you want to restore the default preferences?\n"
								  "The default preferences will be applied immediately.");
	if (b == QMessageBox::Yes) {
		Config().clear();
		load();
	}
}
