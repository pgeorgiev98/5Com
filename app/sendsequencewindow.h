#ifndef SENDSEQUENCEWINDOW_H
#define SENDSEQUENCEWINDOW_H

#include <QDialog>
#include <QVector>

class SerialPort;

class QGridLayout;
class QScrollArea;
class QCheckBox;
class QSpinBox;
class QMenu;

class SendSequenceWindow : public QDialog
{
	Q_OBJECT
	enum class OperationType { Send, Wait, ChangeDTR, ChangeRTS };
public:
	explicit SendSequenceWindow(SerialPort *port, QWidget *parent = nullptr);

signals:
	void operationsCountChanged(int count);

private slots:
	void onSendClicked();
	void addOperation(OperationType);
	void addOperation(OperationType, int);
	void clearOperations();
	void removeOperation(int i, bool adjustSize = false);
	void executeNextOperation();
	void onActionButtonClicked();
	void cancelSequence();

protected:
	void reject() override;

private:
	struct Operation {
		QWidget *label, *input, *actionButton;
		OperationType type;
		int inputSpan;
	};
	QVector<Operation> m_operations;
	SerialPort *m_port;
	QGridLayout *m_operationsLayout;
	QScrollArea *m_operationsScrollArea;
	QCheckBox *m_sendIndefinitely;
	QSpinBox *m_sequencesCount;
	QPushButton *m_sendButton;
	int m_currentOperation;
	QTimer *m_timer;
	QMenu *m_itemMenu;
	int m_itemMenuIndex;
};

#endif // SENDSEQUENCEWINDOW_H
