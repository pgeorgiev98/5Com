#include "line.h"

Line::Line(Type type, QWidget *parent)
	: QFrame(parent)
{
	if (type == Type::Horizontal)
		setFrameShape(QFrame::HLine);
	else
		setFrameShape(QFrame::VLine);
}
