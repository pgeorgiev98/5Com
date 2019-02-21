#ifndef BYTERECEIVETIMESDIALOG_H
#define BYTERECEIVETIMESDIALOG_H

#include <QDialog>

class QTableWidget;
class QTime;

class ByteReceiveTimesDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ByteReceiveTimesDialog(int height, QWidget *parent = nullptr);

signals:

public slots:
	void display(const QByteArray &bytes);
	void clear();

private:
	QTime *m_startTime;
	QTableWidget *m_table;
	int m_rowHeight;
};

#endif // BYTERECEIVETIMESDIALOG_H
