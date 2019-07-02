#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QFont>
#include <QFontMetrics>

class HexView : public QWidget
{
	Q_OBJECT
public:
	explicit HexView(QWidget *parent = nullptr);

	QString toPlainText() const;

public slots:
	void clear();
	void setData(const QByteArray &data);
	void insertData(const QByteArray &data);

protected:
	void paintEvent(QPaintEvent *) override;
	void mouseMoveEvent(QMouseEvent *) override;
	void mousePressEvent(QMouseEvent *) override;
	void mouseReleaseEvent(QMouseEvent *) override;
	void mouseDoubleClickEvent(QMouseEvent *) override;
	void leaveEvent(QEvent *) override;

private:
	QFont m_font;
	QFontMetrics m_fontMetrics;
	int m_characterWidth;
	int m_cellSize, m_cellPadding;
	QByteArray m_data;
	int m_hoveredIndex;
	int m_selectionStart;
	int m_selectionEnd;
	enum class Selection {
		None = 0,
		Cells,
		CellRows,
		Text,
		TextRows,
	} m_selection;
	bool m_selecting;

	int getHoverCell(const QPoint &mousePos) const;
	int getHoverText(const QPoint &mousePos) const;
};

#endif // HEXVIEW_H
