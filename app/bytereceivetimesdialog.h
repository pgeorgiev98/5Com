#ifndef BYTERECEIVETIMESDIALOG_H
#define BYTERECEIVETIMESDIALOG_H

#include <QDialog>
#include <QTime>

class QTableWidget;

class ByteReceiveTimesDialog : public QDialog
{
	Q_OBJECT
public:
	struct Byte
	{
		int ms;
		unsigned char value;
	};

	explicit ByteReceiveTimesDialog(int height, QWidget *parent = nullptr);
	int bytesCount() const;
	const QVector<Byte> &bytes() const;

public slots:
	void removeFromBegining(int bytesCount);
	void insertData(const QByteArray &data);
	void clear();

private:
	QTime m_startTime;
	QTableWidget *m_table;
	int m_rowHeight;
	QVector<Byte> m_bytes;
};

#endif // BYTERECEIVETIMESDIALOG_H
