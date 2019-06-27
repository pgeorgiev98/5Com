#ifndef CHECKFORUPDATESDIALOG_H
#define CHECKFORUPDATESDIALOG_H

#include <QDialog>
#include "latestreleasechecker.h"

class QLabel;
class QStackedLayout;

class CheckForUpdatesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit CheckForUpdatesDialog(const LatestReleaseChecker::Release *release, QWidget *parent = nullptr);

private slots:
	void onUpdateClicked();
	void onFailedToGetLatestRelease(const QString &errorString);
	void onLatestReleaseFound(const LatestReleaseChecker::Release &release);

private:
	LatestReleaseChecker *m_latestReleaseChecker;
	LatestReleaseChecker::Release m_latestRelease;
	QPushButton *m_updateButton;
	QStackedLayout *m_messageLayout;
	QLabel *m_latestReleaseLabel;
};

#endif // CHECKFORUPDATESDIALOG_H
