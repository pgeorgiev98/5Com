#ifndef PLAINTEXTVIEW_H
#define PLAINTEXTVIEW_H

#include <QWidget>
#include <QByteArray>
#include <QString>

class QPlainTextEdit;

class PlainTextView : public QWidget
{
	Q_OBJECT
public:
	explicit PlainTextView(QWidget *parent = nullptr);
	QString toPlainText() const;

signals:

public slots:
	void insertData(const QByteArray &data);
	void clear();

private:
	QPlainTextEdit *m_edit;
	unsigned char m_lastChar;
};

#endif // PLAINTEXTVIEW_H
