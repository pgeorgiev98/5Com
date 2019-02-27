#include <QtTest>
#include "latestreleasechecker.h"

class Versions : public QObject
{
	Q_OBJECT

private slots:
	void testNewerVersions();
};

void Versions::testNewerVersions()
{
	typedef LatestReleaseChecker::Release Release;

	// Newer versions
	QVERIFY(Release("0.1.0").isNewerThan("0.0.0"));
	QVERIFY(Release("0.1.0").isNewerThan("0.0.1"));
	QVERIFY(Release("0.1.0").isNewerThan("0.0.23"));

	QVERIFY(Release("0.2.0").isNewerThan("0.0.0"));
	QVERIFY(Release("0.2.0").isNewerThan("0.0.4"));
	QVERIFY(Release("0.2.0").isNewerThan("0.1.0"));
	QVERIFY(Release("0.2.0").isNewerThan("0.1.55"));

	QVERIFY(Release("3.8.2").isNewerThan("0.0.0"));
	QVERIFY(Release("3.8.2").isNewerThan("2.9.9"));
	QVERIFY(Release("3.8.2").isNewerThan("3.7.9"));
	QVERIFY(Release("3.8.2").isNewerThan("3.8.1"));

	QVERIFY(Release("0.1.0-rc1").isNewerThan("0.1.0"));
	QVERIFY(Release("0.1.0-rc2").isNewerThan("0.1.0-rc1"));
	QVERIFY(Release("1.0.0").isNewerThan("0.1.1-rc1"));
	QVERIFY(Release("2.1.0-beta").isNewerThan("2.1.0-alpha"));
	QVERIFY(Release("2.1.0-beta.3").isNewerThan("2.1.0-beta.2"));
	QVERIFY(Release("1.9.0-alpha").isNewerThan("1.8.2-beta.2"));

	// Older versions
	QVERIFY(!Release("0.0.0").isNewerThan("0.1.0"));
	QVERIFY(!Release("0.0.1").isNewerThan("0.1.0"));
	QVERIFY(!Release("0.0.23").isNewerThan("0.1.0"));

	QVERIFY(!Release("0.0.0").isNewerThan("0.2.0"));
	QVERIFY(!Release("0.0.4").isNewerThan("0.2.0"));
	QVERIFY(!Release("0.1.0").isNewerThan("0.2.0"));
	QVERIFY(!Release("0.1.55").isNewerThan("0.2.0"));

	QVERIFY(!Release("0.0.0").isNewerThan("3.8.2"));
	QVERIFY(!Release("2.9.9").isNewerThan("3.8.2"));
	QVERIFY(!Release("3.7.9").isNewerThan("3.8.2"));
	QVERIFY(!Release("3.8.1").isNewerThan("3.8.2"));

	QVERIFY(!Release("0.1.0").isNewerThan("0.1.0-rc1"));
	QVERIFY(!Release("0.1.0-rc1").isNewerThan("0.1.0-rc2"));
	QVERIFY(!Release("0.1.1-rc1").isNewerThan("1.0.0"));
	QVERIFY(!Release("2.1.0-alpha").isNewerThan("2.1.0-beta"));
	QVERIFY(!Release("2.1.0-beta.2").isNewerThan("2.1.0-beta.3"));
	QVERIFY(!Release("1.8.3-beta.3").isNewerThan("1.9.0-alpha"));

	// Same versions
	QVERIFY(!Release("0.1.0").isNewerThan("0.1.0"));
	QVERIFY(!Release("1.9.2").isNewerThan("1.9.2"));
	QVERIFY(!Release("5.3.4-alpha").isNewerThan("5.3.4-alpha"));
}

QTEST_APPLESS_MAIN(Versions)

#include "versions.moc"
