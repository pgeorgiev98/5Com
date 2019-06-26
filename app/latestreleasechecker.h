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
		QString windowsDownloadUrl;

		Release(const QString &versionString, const QString &url,
				const QString &windowsDownloadUrl)
			: versionString(versionString)
			, url(url)
			, windowsDownloadUrl(windowsDownloadUrl)
		{}

		Release(const QString &versionString)
			: versionString(versionString)
		{}

		bool isNewerThan(const QString &other) const
		{
			QRegExp sep("\\.-");
			QStringList l1 = versionString.split(sep);
			QStringList l2 = other.split(sep);

			int minLen = qMin(l1.size(), l2.size());

			for (int i = 0; i < minLen; ++i) {
				bool ok1, ok2;
				int a = l1[i].toInt(&ok1);
				int b = l2[i].toInt(&ok2);
				if (ok1 && ok2) {
					if (a > b)
						return true;
					else if (a < b)
						return false;
				} else {
					if (l1[i] > l2[i])
						return true;
				}
			}

			if (l1.size() > l2.size())
				return true;

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
