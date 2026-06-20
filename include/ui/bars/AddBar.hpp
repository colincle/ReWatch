#pragma once

#include "IconButton.hpp"
#include "SearchBar.hpp"

#include <QWidget>

class AddBar : public QWidget
{
	Q_OBJECT

  public:
	explicit AddBar(QWidget *parent = nullptr);

  signals:
	void requestNormalMode();
	void searchRequested(const QString &query);

  protected:
	void showEvent(QShowEvent *event) override;

  public:
	void refreshStyle();

  private:
	IconButton *returnButton;
	SearchBar  *searchBar;

	void setupLayout();
	void connectSignals();

	void onReturnClicked();
	void onSearchCommitted();
};