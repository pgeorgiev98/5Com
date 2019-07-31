#include "latestreleasechecker.h"
#include "common.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

LatestReleaseChecker::LatestReleaseChecker(QObject *parent)
	: QObject(parent)
	, m_manager(new QNetworkAccessManager(this))
	, m_latestRelease(Release(VERSION))
{
	connect(m_manager, &QNetworkAccessManager::finished,
			this, &LatestReleaseChecker::onRequestFinished);
}

const LatestReleaseChecker::Release &LatestReleaseChecker::latestRelease() const
{
	return m_latestRelease;
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

	QString windowsDownloadUrl;

	auto assetsRef = rootObject.find("assets");
	if (assetsRef->isArray()) {
		auto assetsArr = assetsRef->toArray();
		for (int i = 0; i < assetsArr.count(); ++i) {
			auto assetRef = assetsArr[i];
			if (!assetRef.isObject())
				continue;
			auto asset = assetRef.toObject();
			auto name = asset.find("name");
			if (!name->isString())
				continue;
			auto browserDownloadUrl = asset.find("browser_download_url");
			if (!browserDownloadUrl->isString())
				continue;
			if (!name->toString().contains("win32"))
				continue;

			windowsDownloadUrl = browserDownloadUrl->toString();
			break;
		}
	}

	QString version = tagNameRef.toString();
	QString url = htmlUrlRef.toString();

	if (!version.isEmpty() && version[0] == 'v')
		version.remove(0, 1);

	m_latestRelease = Release(version, url, windowsDownloadUrl);
	emit latestReleaseFound(m_latestRelease);
}
