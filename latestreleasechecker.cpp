#include "latestreleasechecker.h"
#include "common.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <QDebug>

LatestReleaseChecker::LatestReleaseChecker(QObject *parent)
	: QObject(parent)
	, m_manager(new QNetworkAccessManager(this))
{
	connect(m_manager, &QNetworkAccessManager::finished,
			this, &LatestReleaseChecker::onRequestFinished);
}

void LatestReleaseChecker::checkLatestRelease()
{
	QNetworkRequest request;
	request.setUrl(QUrl(LATEST_RELEASE_URL));
	m_manager->get(request);
}

void LatestReleaseChecker::onRequestFinished(QNetworkReply *reply)
{
	static const char *errorHeader = "Failed to get latest release info:";
	if (reply->error()) {
		qWarning() << errorHeader << reply->errorString();
		emit failedToGetLatestRelease(reply->errorString());
		return;
	}
	QString answer = reply->readAll();
	QJsonParseError error;
	QJsonDocument document = QJsonDocument::fromJson(answer.toUtf8(), &error);
	static const char *parseErrorString = "Response parse error";
	if (error.error != QJsonParseError::NoError) {
		qWarning() << errorHeader << error.errorString();
		emit failedToGetLatestRelease(parseErrorString);
		return;
	}
	QJsonObject rootObject = document.object();

	auto tagNameIter = rootObject.find("tag_name");
	if (tagNameIter == rootObject.end()) {
		qWarning() << errorHeader << "\"tag_name\" not present";
		emit failedToGetLatestRelease(parseErrorString);
		return;
	}
	auto tagNameRef = tagNameIter.value();
	if (!tagNameRef.isString()) {
		qWarning() << errorHeader << "\"tag_name\" value is not a string";
		emit failedToGetLatestRelease(parseErrorString);
		return;
	}

	auto htmlUrl = rootObject.find("html_url");
	if (htmlUrl == rootObject.end()) {
		qWarning() << errorHeader << "\"html_url\" not present";
		emit failedToGetLatestRelease(parseErrorString);
		return;
	}
	auto htmlUrlRef = htmlUrl.value();
	if (!htmlUrlRef.isString()) {
		qWarning() << errorHeader << "\"html_url\" value is not a string";
		emit failedToGetLatestRelease(parseErrorString);
		return;
	}

	QString version = tagNameRef.toString();
	QString url = htmlUrlRef.toString();

	if (!version.isEmpty() && version[0] == 'v')
		version.remove(0, 1);

	Release release;
	release.versionString = version;
	release.url = url;
	emit latestReleaseFound(release);
}
