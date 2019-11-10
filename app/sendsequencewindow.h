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
	void moveOperation(int before, int after);
	void clearOperations();
	void removeOperation(int i, bool adjustSize = false);
	void executeNextOperation();
	void onActionButtonClicked();
	void cancelSequence();
	void saveSequence();
	void loadSequence();
	void loadSequence(const QString &filePath);
	void addRecent(const QString &path);

protected:
	void reject() override;

private:
	struct Operation {
		QWidget *label, *input, *actionButton;
		OperationType type;
		int inputSpan;

		Operation(OperationType type, int inputSpan)
			: label(nullptr), input(nullptr), actionButton(nullptr)
			, type(type), inputSpan(inputSpan) {}

		Operation() : Operation(OperationType::Send, 0) {}
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
	QMenu *m_recents;
	QString m_sequencesDirectory;
};

#endif // SENDSEQUENCEWINDOW_H
