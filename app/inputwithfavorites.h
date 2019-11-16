#ifndef INPUTWITHFAVORITES_H
#define INPUTWITHFAVORITES_H

#include <QWidget>

#include <variant>

class QLineEdit;
class InputField;
class QToolButton;

class InputWithFavorites : public QWidget
{
	Q_OBJECT
public:
	enum ButtonsPlacement { Left, Right };
	explicit InputWithFavorites(ButtonsPlacement buttonsPlacement, std::variant<QLineEdit *, InputField *> inputWidget, QWidget *parent = nullptr);

	QString text() const;

public slots:
	void setText(const QString &text);
	void focusInput();

private slots:
	void onTextChanged(const QString &text);

private:
	static QIcon isNotFavoriteIcon;
	static QIcon isFavoriteIcon;
	static QIcon chooseFromFavoritesIcon;

	std::variant<QLineEdit *, InputField *> m_inputWidget;
	QToolButton *m_saveButton;
	QToolButton *m_chooseButton;
};

#endif // INPUTWITHFAVORITES_H
