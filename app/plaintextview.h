#ifndef PLAINTEXTVIEW_H
#define PLAINTEXTVIEW_H

#include <QWidget>
#include <QByteArray>
#include <QString>
#include <QFont>
#include <QFontMetricsF>

#include <optional>

#include "byteselection.h"

class PlainTextView : public QWidget
{
	Q_OBJECT
public:
	explicit PlainTextView(QWidget *parent = nullptr);
	QString toPlainText() const;
	QPoint getByteCoordinates(int index) const;
	std::optional<ByteSelection> selection() const;

public slots:
	void setData(const QByteArray &data);
	void insertData(const QByteArray &data);
	void clear();
	void copySelection();
	void selectAll();
	void selectNone();
	void highlight(ByteSelection selection);
	void setFont(QFont font);

signals:
	void highlightInHexView(ByteSelection selection);

protected:
	void paintEvent(QPaintEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void leaveEvent(QEvent *event) override;

private:
	struct ElementId
	{
		int row, element, index;
		ElementId() : row(-1), element(-1), index(0) {}
		ElementId(int row, int element, int index)
			: row(row), element(element), index(index) {}
		bool operator==(const ElementId &other) const
		{
			return row == other.row && element == other.element && index == other.index;
		}
		bool operator>(const ElementId &other) const
		{
			return row > other.row || (row == other.row && element > other.element) || (row == other.row && element == other.element && index > other.index);
		}
		bool operator<(const ElementId &other) const
		{
			return row < other.row || (row == other.row && element < other.element) || (row == other.row && element == other.element && index < other.index);
		}
		bool operator>=(const ElementId &other) const
		{
			return *this > other || *this == other;
		}
		bool operator<=(const ElementId &other) const
		{
			return *this < other || *this == other;
		}
	};

	struct Element
	{
		enum Type
		{
			PlainText = 0,
			StandardHexCode = 1,
			NonStandardHexCode = 2,
		};

		int rawStartIndex;
		int type;
		QString str;
	};

	struct ByteInfo
	{
		int type;
		QString str;
		QString name;
	};

	struct Row
	{
		QVector<Element> elements;
	};

	QFont m_font;
	QFontMetricsF m_fm;
	QByteArray m_data;
	int m_width, m_height;
	int m_padding;
	QPoint m_mousePos;
	std::optional<QPoint> m_mousePressPos;
	int m_pressedByteIndex;
	std::optional<ByteSelection> m_selection;

	QVector<Row> m_rows;

	static const ByteInfo byteInfos[256];

	int getByteIndexAtPos(QPoint pos, bool selecting = false) const;
	std::optional<ByteSelection> getSelectedBytes() const;
	void recalculateSize();
};

#endif // PLAINTEXTVIEW_H
