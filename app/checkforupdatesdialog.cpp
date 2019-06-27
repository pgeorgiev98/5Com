#include "checkforupdatesdialog.h"
#include "common.h"

#ifdef Q_OS_WIN
#include "autoupdatedialog.h"
#endif

#include <QVBoxLayout>
#include <QStackedLayout>

#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

CheckForUpdatesDialog::CheckForUpdatesDialog(const LatestReleaseChecker::Release *release, QWidget *parent)
	: QDialog(parent)
	, m_latestReleaseChecker(new LatestReleaseChecker(this))
	, m_latestRelease(LatestReleaseChecker::Release(VERSION))
#ifdef Q_OS_WIN
	, m_updateButton(new QPushButton("Update"))
#else
	, m_updateButton(nullptr)
#endif
	, m_messageLayout(new QStackedLayout)
	, m_latestReleaseLabel(new QLabel)
{
	if (m_updateButton)
		m_updateButton->setEnabled(false);
	m_latestReleaseLabel->setFixedSize(300, 80);
	m_latestReleaseLabel->setWordWrap(true);
	m_latestReleaseLabel->setOpenExternalLinks(true);

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	QPushButton *closeButton = new QPushButton("Close");

	layout->addWidget(new QLabel(QString("You're currently using version <b>%1</b>.").arg(VERSION)));
	layout->addLayout(m_messageLayout);
	{
		QHBoxLayout *hbox = new QHBoxLayout;
		hbox->addStretch(1);
		hbox->addWidget(closeButton);
		if (m_updateButton) {
			hbox->addStretch(1);
			hbox->addWidget(m_updateButton);
		}
		hbox->addStretch(1);
		layout->addLayout(hbox);
	}

	QWidget *loadingWidget = new QWidget;
	QHBoxLayout *loadingLayout = new QHBoxLayout;
	QProgressBar *progressBar = new QProgressBar;
	loadingLayout->setMargin(0);
	progressBar->setRange(0, 0);
	loadingLayout->addWidget(new QLabel("Checking latest version"));
	loadingLayout->addWidget(progressBar);
	loadingWidget->setLayout(loadingLayout);

	m_messageLayout->addWidget(loadingWidget);
	m_messageLayout->addWidget(m_latestReleaseLabel);

	m_messageLayout->setCurrentIndex(0);

	connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
	if (m_updateButton)
		connect(m_updateButton, &QPushButton::clicked, this, &CheckForUpdatesDialog::onUpdateClicked);
	connect(m_latestReleaseChecker, &LatestReleaseChecker::failedToGetLatestRelease,
			this, &CheckForUpdatesDialog::onFailedToGetLatestRelease);
	connect(m_latestReleaseChecker, &LatestReleaseChecker::latestReleaseFound,
			this, &CheckForUpdatesDialog::onLatestReleaseFound);

	if (release) {
		onLatestReleaseFound(*release);
	} else {
		m_latestReleaseChecker->checkLatestRelease();
	}
}

void CheckForUpdatesDialog::onUpdateClicked()
{
#ifdef Q_OS_WIN
	accept();
	AutoUpdateDialog dialog(parentWidget());
	dialog.startDownloading(m_latestRelease.windowsDownloadUrl);
	dialog.exec();
#endif
}

void CheckForUpdatesDialog::onFailedToGetLatestRelease(const QString &errorString)
{
	m_latestReleaseLabel->setText("Error: " + errorString);
	m_messageLayout->setCurrentIndex(1);
}

void CheckForUpdatesDialog::onLatestReleaseFound(const LatestReleaseChecker::Release &release)
{
	m_latestRelease = release;
	if (release.isNewerThan(VERSION)) {
		m_latestReleaseLabel->setText(QString(
										"Latest release is version <b>%1</b>.<br/>"
										"You can manually download it from <a href=\"%2\">here</a>%3")
										.arg(release.versionString)
										.arg(release.url)
							  #ifdef Q_OS_WIN
										.arg(release.windowsDownloadUrl.isEmpty() ?
												 ".<br/>Automatic updating is currently not available." :
												 " or update automatically by pressing the 'Update' button."));
							  #else
										.arg("."));
							  #endif
	} else {
		m_latestReleaseLabel->setText(QString(
										"Latest release is version <b>%1</b>.<br/>"
										"You are up to date.").arg(release.versionString));
	}
	m_messageLayout->setCurrentIndex(1);
	if (m_updateButton)
		m_updateButton->setEnabled(!release.windowsDownloadUrl.isEmpty());
}
