#include "escapecodesdialog.h"
#include "common.h"
#include <QHBoxLayout>
#include <QLabel>

EscapeCodesDialog::EscapeCodesDialog(QWidget *parent)
	: QDialog(parent)
{
	QString text = "\
Escape codes:\n\n\
\\\\   - Backslash (\\)\n\
\\n   - Linefeed (LF)\n\
\\r   - Carriage return (CR)\n\
\\t   - Horizontal Tab\n\
\\xHH - Byte with a hex value of HH\
";
	QHBoxLayout *layout = new QHBoxLayout;
	QLabel *label = new QLabel;
	label->setText(text);
	label->setFont(getFixedFont());
	label->setWordWrap(true);
	label->setTextFormat(Qt::TextFormat::PlainText);
	layout->addWidget(label);

	setLayout(layout);
}
