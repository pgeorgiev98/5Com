#ifndef LATESTRELEASECHECKER_H
#define LATESTRELEASECHECKER_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class LatestReleaseChecker : public QObject
{
	Q_OBJECT
public:
	explicit LatestReleaseChecker(QObject *parent = nullptr);

	struct Release
	{
		QString versionString, url;
	};

signals:
	void failedToGetLatestRelease(const QString &errorMessage);
	void latestReleaseFound(const Release &latestRelease);

public slots:
	void checkLatestRelease();

private slots:
	void onRequestFinished(QNetworkReply *reply);

private:
	QNetworkAccessManager *m_manager;
};

#endif // LATESTRELEASECHECKER_H
