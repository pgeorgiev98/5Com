#include "favoritesselectionlist.h"
#include "favoriteslistobject.h"

#include <QVBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QMenu>

FavoritesSelectionList::FavoritesSelectionList(QWidget *parent)
	: QDialog(parent)
	, m_list(new QListWidget)
{
	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(new QLabel("Select from favorites:"), 0, Qt::AlignHCenter);
	layout->addSpacing(8);
	layout->addWidget(m_list);

	FavoritesListObject *o = FavoritesListObject::instance();
	m_list->addItems(o->favorites());
	setLayout(layout);

	m_list->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_list, &QListWidget::customContextMenuRequested, [this, o](QPoint point) {
		if (m_list->selectedItems().isEmpty())
			return;
		QAction removeAction("Remove");
		QMenu menu(this);
		menu.addAction(&removeAction);
		menu.popup(m_list->mapToGlobal(point));
		if (menu.exec() == &removeAction)
			o->removeFavorite(m_list->selectedItems().first()->text());
	});

	connect(m_list, &QListWidget::itemActivated, [this](QListWidgetItem *item) {
		m_selectedString = item->text();
		accept();
	});

	connect(o, &FavoritesListObject::favoriteAdded, this, &FavoritesSelectionList::updateList);
	connect(o, &FavoritesListObject::favoriteRemoved, this, &FavoritesSelectionList::updateList);

	QFontMetricsF fm = QFontMetricsF(m_list->font());
	resize(int(fm.averageCharWidth() * 80), int(fm.height() * 20));
}

std::optional<QString> FavoritesSelectionList::selectedString() const
{
	return m_selectedString;
}

void FavoritesSelectionList::updateList()
{
	m_list->clear();
	m_list->addItems(FavoritesListObject::instance()->favorites());
}
