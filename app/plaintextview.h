#ifndef PLAINTEXTVIEW_H
#define PLAINTEXTVIEW_H

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QFont>
#include <QFontMetrics>

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

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	QFont m_font;
	QFontMetrics m_fm;
	QByteArray m_data;
	int m_width, m_height;
	int m_padding;

	struct Element
	{
		enum Type
		{
			PlainText = 0,
			StandardHexCode = 1,
			NonStandardHexCode = 2,
		};

		int type;
		QString str;
	};

	static const Element byteInfos[256];

	struct Row
	{
		QVector<Element> elements;
	};

	QVector<Row> m_rows;
};

#endif // PLAINTEXTVIEW_H
