#ifndef CONFIG_H
#define CONFIG_H

#include <QVariant>
#include <QSettings>
#include <QStringList>

#ifdef Q_OS_WIN
#define BUILDING_FOR_WINDOWS true
#else
#define BUILDING_FOR_WINDOWS false
#endif

#define EXPORT_SEPARATOR_STRINGS QStringList({", (comma)", "; (semicolon)", "<Tab>", "<Space>"})
#define EXPORT_SEPARATORS QStringList({",", ";", "\t", " "})

#define FIELD(TYPE, CONV, NAME, SETTER, KEY, DEFAULT) \
	TYPE NAME() const { return m_settings.value(KEY, DEFAULT).to ## CONV(); } \
	void SETTER(const TYPE &NAME ## _) { m_settings.setValue(KEY, NAME ## _); }

class Config
{
public:
	Config() {}
	Config(const QString filePath)
		: m_settings(filePath, QSettings::IniFormat) {}

	FIELD(bool, Bool, includePtsDirectory, setIncludePtsDirectory, "includePtsDirectory", false)
	FIELD(bool, Bool, checkForUpdatesOnStartup, setCheckForUpdatesOnStartup, "checkForUpdatesOnStartup", true)
	FIELD(int, Int, readBufferLimitKiB, setReadBufferLimitKiB, "readBufferLimitKiB", 100)
	FIELD(QString, String, lastExportDirectory, setLastExportDirectory, "lastExportDirectory", "")
	FIELD(bool, Bool, exportWithCRLFEndings, setExportWithCRLFEndings, "exportWithCRLFEndings", BUILDING_FOR_WINDOWS)
	FIELD(bool, Bool, exportWithSepStatement, setExportWithSepStatement, "exportWithSepStatement", false)
	FIELD(QString, String, exportSeparator, setExportSeparator, "exportSeparator", ",")
	FIELD(int, Int, inputHistoryLength, setInputHistoryLength, "inputHistoryLength", 10)

	QSettings &settings() { return m_settings; }
	QString path() const { return m_settings.fileName(); }
private:
	QSettings m_settings;
};

#undef FIELD

#endif // CONFIG_H
