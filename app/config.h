#ifndef CONFIG_H
#define CONFIG_H

#include <QList>
#include <QVariant>
#include <QSettings>

#include "common.h"

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

	QSettings &settings() { return m_settings; }
	QString path() const { return m_settings.fileName(); }
private:
	QSettings m_settings;
};

#undef FIELD

#endif // CONFIG_H
