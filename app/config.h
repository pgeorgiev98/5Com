#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

#include <QVariant>
#include <QSettings>
#include <QStringList>
#include <QSize>
#include <QKeySequence>

#ifdef Q_OS_WIN
#define BUILDING_FOR_WINDOWS true
#else
#define BUILDING_FOR_WINDOWS false
#endif

#define EXPORT_SEPARATOR_STRINGS QStringList({", (comma)", "; (semicolon)", "<Tab>", "<Space>"})
#define EXPORT_SEPARATORS QStringList({",", ";", "\t", " "})

#define FIELD(TYPE, CONV, NAME, SETTER, KEY, DEFAULT) \
	public: \
	TYPE NAME() const { return m_settings.value(KEY, DEFAULT).to ## CONV(); } \
	void SETTER(const TYPE &NAME ## _) { m_settings.setValue(KEY, NAME ## _); } \
	private: \
	Field _##NAME = Field(KEY, DEFAULT);

class Config
{
private:
	struct Field
	{
		QString key;
		QVariant defaultValue;
		Field(const QString &key, QVariant defaultValue)
			: key(key), defaultValue(defaultValue) {}
	};
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
	FIELD(bool, Bool, clearInputOnSend, setClearInputOnSend, "clearInputOnSend", false)
	FIELD(bool, Bool, rememberLastUsedPort, setRememberLastUsedPort, "rememberLastUsedPort", true)
	FIELD(QString, String, lastUsedPort, setLastUsedPort, "lastUsedPort", QString())
	FIELD(QString, String, lastUsedPortBaudRate, setLastUsedPortBaudRate, "lastUsedPortBaudRate", "9600")
	FIELD(QString, String, lastUsedPortDataBits, setLastUsedPortDataBits, "lastUsedPortDataBits", "8")
	FIELD(QString, String, lastUsedPortParity, setLastUsedPortParity, "lastUsedPortParity", "None")
	FIELD(QString, String, lastUsedPortStopBits, setLastUsedPortStopBits, "lastUsedPortStopBits", "1")
	FIELD(int, Int, hexViewBytesPerLine, setHexViewBytesPerLine, "hexViewBytesPerLine", 16)
	FIELD(bool, Bool, saveInputHistory, setSaveInputHistory, "saveInputHistory", true)
	FIELD(QStringList, StringList, inputHistory, setInputHistory, "inputHistory", QStringList())
	FIELD(QSize, Size, mainWindowSize, setMainWindowSize, "mainWindowSize", QSize(640, 480))
	FIELD(bool, Bool, saveMainWindowSize, setSaveMainWindowSize, "saveMainWindowSize", true)
	FIELD(QStringList, StringList, recentSequences, setRecentSequences, "recentSequences", QStringList())
	FIELD(QStringList, StringList, favoriteInputs, setFavoriteInputs, "favoriteInputs", QStringList())

public:
	class Font
	{
		FIELD(bool, Bool, useBuildInFixedFont, setUseBuildInFixedFont, "useBuildInFixedFont", BUILDING_FOR_WINDOWS)
		FIELD(bool, Bool, useSystemFixedFont, setUseSystemFixedFont, "useSystemFixedFont", !BUILDING_FOR_WINDOWS)
		FIELD(QString, String, fixedFontName, setFixedFontName, "fixedFontName", QString())
		FIELD(int, Int, fixedFontSize, setFixedFontSize, "fixedFontSize", getDefaultFixedFontSize())

	public:
		void clear()
		{
			const int count = (sizeof(Font) - sizeof(QSettings)) / sizeof(Field);
			Field *f = reinterpret_cast<Field *>(this);
			for (int i = 0; i < count; ++i)
				m_settings.remove(f[i].key);
		}

	private:
		QSettings m_settings;
	} font;

public:
	class Shortcuts
	{
		FIELD(QString, String, exportShortcut, setExportShortcut, "exportShortcut", QKeySequence(Qt::CTRL + Qt::Key_E).toString())
		FIELD(QString, String, clearScreenShortcut, setClearScreenShortcut, "clearScreenShortcut", QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L).toString())
		FIELD(QString, String, writeFileShortcut, setWriteFileShortcut, "writeFileShortcut", QKeySequence(QKeySequence::Open).toString())
		FIELD(QString, String, quitShortcut, setQuitShortcut, "quitShortcut", QKeySequence(QKeySequence::Quit).toString())

		FIELD(QString, String, focusInputShortcut, setFocusInputShortcut, "focusInputShortcut", QKeySequence(Qt::CTRL + Qt::Key_L).toString())
		FIELD(QString, String, clearInputFieldShortcut, setClearInputFieldShortcut, "clearInputFieldShortcut", QKeySequence(Qt::CTRL + Qt::Key_W).toString())
		FIELD(QString, String, openPlainTextViewShortcut, setOpenPlainTextViewShortcut, "openPlainTextViewShortcut", QKeySequence(Qt::ALT + Qt::Key_1).toString())
		FIELD(QString, String, openHexViewShortcut, setOpenHexViewShortcut, "openHexViewShortcut", QKeySequence(Qt::ALT + Qt::Key_2).toString())
		FIELD(QString, String, connectToPortShortcut, setConnectToPortShortcut, "connectToPortShortcut", QKeySequence(Qt::CTRL + Qt::Key_Return).toString())

	public:
		void clear()
		{
			const int count = (sizeof(Shortcuts) - sizeof(QSettings)) / sizeof(Field);
			Field *f = reinterpret_cast<Field *>(this);
			for (int i = 0; i < count; ++i)
				m_settings.remove(f[i].key);
		}

	private:
		QSettings m_settings;
	} shortcuts;

	QSettings &settings() { return m_settings; }
	QString path() const { return m_settings.fileName(); }

	void clear()
	{
		const int count = (sizeof(Config) - sizeof(Font) - sizeof(Shortcuts) - sizeof(QSettings)) / sizeof(Field);
		Field *f = reinterpret_cast<Field *>(this);
		for (int i = 0; i < count; ++i)
			m_settings.remove(f[i].key);
	}
private:
	QSettings m_settings;
};

#undef FIELD

#endif // CONFIG_H
