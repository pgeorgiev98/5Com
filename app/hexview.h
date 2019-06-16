#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QWidget>

class QPlainTextEdit;

class HexView : public QWidget
{
public:
	HexView(QWidget *parent = nullptr);
	QString toPlainText() const;

public slots:
	void setData(const QByteArray &data);
	void insertData(const QByteArray &data);
	void clear();

private:
	QPlainTextEdit *m_plainTextEdit;
	QString m_text;
	quint64 m_bytes;
};

#endif // HEXVIEW_H
