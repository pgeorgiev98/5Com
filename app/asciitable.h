#ifndef ASCIITABLE_H
#define ASCIITABLE_H

#include <QDialog>

class AsciiTable : public QDialog
{
	Q_OBJECT
public:
	explicit AsciiTable(QWidget *parent = nullptr);
};

#endif // ASCIITABLE_H
