// Searchable help dialog that renders all items from HelpItems.hpp.
#pragma once

#include <QDialog>
#include <QVector>

class QLineEdit;
class QVBoxLayout;
class QWidget;

class HelpDialog : public QDialog
{
	Q_OBJECT

  public:
	explicit HelpDialog(QWidget *parent = nullptr);

  private:
	QLineEdit   *searchInput = nullptr;
	QWidget     *listContainer = nullptr;
	QVBoxLayout *listLayout = nullptr;

	struct ItemWidget
	{
		QWidget *widget;
		QString  searchText;
	};
	QVector<ItemWidget> items;

	void buildItems();
	void onSearchChanged(const QString &query);
};
