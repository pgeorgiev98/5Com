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

	static LatestReleaseChecker *instance();

	struct Release
	{
		QString versionString, url;

		bool isNewerThan(const QString &other) const
		{
			QStringList l1 = versionString.split('.');
			QStringList l2 = other.split('.');
			if (l1.size() != 3)
				return true;
			if (l2.size() != 3)
				return false;

			for (int i = 0; i < 3; ++i) {
				int a = l1[i].toInt();
				int b = l2[i].toInt();
				if (a > b)
					return true;
				else if (a < b)
					return false;
			}
			return false;
		}
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
