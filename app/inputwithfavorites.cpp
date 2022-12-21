#include "inputwithfavorites.h"
#include "inputfield.h"
#include "favoritesselectionlist.h"
#include "favoriteslistobject.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

static QWidget *widget(std::variant<QLineEdit *, InputField *> v)
{
	if (std::holds_alternative<QLineEdit *>(v))
		return std::get<QLineEdit *>(v);
	else
		return std::get<InputField *>(v);
}

const auto savedToolTip = "Remove from favorites";
const auto notSavedToolTip = "Add to favorites";

QIcon InputWithFavorites::isNotFavoriteIcon = QIcon();
QIcon InputWithFavorites::isFavoriteIcon = QIcon();
QIcon InputWithFavorites::chooseFromFavoritesIcon = QIcon();

InputWithFavorites::InputWithFavorites(ButtonsPlacement buttonsPlacement, std::variant<QLineEdit *, InputField *> inputWidget, QWidget *parent)
	: QWidget(parent)
	, m_inputWidget(inputWidget)
	, m_saveButton(new QToolButton)
	, m_chooseButton(new QToolButton)
{
	if (isNotFavoriteIcon.isNull()) {
		if (palette().base().color().lightnessF() > 0.5) {
			isNotFavoriteIcon = QIcon(":/is_not_favorite_icon_light.png");
			isFavoriteIcon = QIcon(":/is_favorite_icon_light.png");
			chooseFromFavoritesIcon = QIcon(":/choose_favorite_icon_light.png");
		} else {
			isNotFavoriteIcon = QIcon(":/is_not_favorite_icon_dark.png");
			isFavoriteIcon = QIcon(":/is_favorite_icon_dark.png");
			chooseFromFavoritesIcon = QIcon(":/choose_favorite_icon_dark.png");
		}
	}

	m_saveButton->setAutoRaise(true);
	m_chooseButton->setAutoRaise(true);

	m_chooseButton->setIcon(chooseFromFavoritesIcon);

	m_chooseButton->setToolTip("Choose from favorites");

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setMargin(2);
	layout->setSpacing(2);
	if (buttonsPlacement == Left) {
		layout->addWidget(m_chooseButton);
		layout->addWidget(m_saveButton);
		layout->addWidget(widget(m_inputWidget));
	} else {
		layout->addWidget(widget(m_inputWidget));
		layout->addWidget(m_saveButton);
		layout->addWidget(m_chooseButton);
	}
	setLayout(layout);

	FavoritesListObject *favoritesListObject = FavoritesListObject::instance();
	connect(favoritesListObject, &FavoritesListObject::favoriteAdded, [this](QString value) {
		if (value == text()) {
			m_saveButton->setIcon(isFavoriteIcon);
			m_saveButton->setToolTip(savedToolTip);
		}
	});
	connect(favoritesListObject, &FavoritesListObject::favoriteRemoved, [this](QString value) {
		if (value == text()) {
			m_saveButton->setIcon(isNotFavoriteIcon);
			m_saveButton->setToolTip(notSavedToolTip);
		}
	});

	connect(m_saveButton, &QToolButton::clicked, [this]() {
		FavoritesListObject *o = FavoritesListObject::instance();
		if (o->isFavorite(text()))
			o->removeFavorite(text());
		else
			o->addFavorite(text());
	});

	connect(m_chooseButton, &QToolButton::clicked, [this]() {
		FavoritesSelectionList l(this);
		if (l.exec())
			setText(*l.selectedString());
	});

	onTextChanged(text());

	if (std::holds_alternative<QLineEdit *>(inputWidget))
		connect(std::get<QLineEdit *>(inputWidget), &QLineEdit::textChanged, this, &InputWithFavorites::onTextChanged);
	else
		connect(std::get<InputField *>(inputWidget), &InputField::currentTextChanged, this, &InputWithFavorites::onTextChanged);
}

QString InputWithFavorites::text() const
{
	if (std::holds_alternative<QLineEdit *>(m_inputWidget))
		return std::get<QLineEdit *>(m_inputWidget)->text();
	else
		return std::get<InputField *>(m_inputWidget)->currentText();
}

void InputWithFavorites::setText(const QString &text)
{
	if (std::holds_alternative<QLineEdit *>(m_inputWidget))
		std::get<QLineEdit *>(m_inputWidget)->setText(text);
	else
		std::get<InputField *>(m_inputWidget)->setCurrentText(text);
}

void InputWithFavorites::focusInput()
{
	if (std::holds_alternative<QLineEdit *>(m_inputWidget))
		std::get<QLineEdit *>(m_inputWidget)->setFocus();
	else
		std::get<InputField *>(m_inputWidget)->setFocus();
}

void InputWithFavorites::onTextChanged(const QString &text)
{
	m_saveButton->setDisabled(text.isEmpty());

	if (FavoritesListObject::instance()->isFavorite(text)) {
		m_saveButton->setIcon(isFavoriteIcon);
		m_saveButton->setToolTip(savedToolTip);
	} else {
		m_saveButton->setIcon(isNotFavoriteIcon);
		m_saveButton->setToolTip(notSavedToolTip);
	}
}
