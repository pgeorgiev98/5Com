#include "mainwindow.h"
#include "common.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setOrganizationName("5Com");
	a.setOrganizationDomain("5Com.com");
	a.setApplicationName("5Com");
	QSettings::setDefaultFormat(QSettings::IniFormat);

	loadBuiltInFont();

	MainWindow w;
	w.show();

	return a.exec();
}
