#ifndef PLAINTEXTVIEW_H
#define PLAINTEXTVIEW_H

#include <QWidget>
#include <QByteArray>
#include <QString>

class QTextBrowser;

class PlainTextView : public QWidget
{
	Q_OBJECT
public:
	explicit PlainTextView(QWidget *parent = nullptr);
	QString toPlainText() const;

public slots:
	void setData(const QByteArray &data);
	void insertData(const QByteArray &data);
	void clear();
	void setColorSpecialCharacters(bool colorSpecialCharacters);

private:
	QTextBrowser *m_edit;
	unsigned char m_lastChar;
	QColor m_defaultTextColor;
	bool m_colorSpecialCharacters;
};

#endif // PLAINTEXTVIEW_H
