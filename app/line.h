#ifndef LINE_H
#define LINE_H

#include <QFrame>

class Line : public QFrame
{
	Q_OBJECT
public:
	enum Type
	{
		Horizontal,
		Vertical,
	};

	explicit Line(Type type, QWidget *parent = nullptr);
};

#endif // LINE_H
