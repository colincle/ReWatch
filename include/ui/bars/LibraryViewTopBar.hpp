#pragma once

#include "IconButton.hpp"
#include "SortEnums.hpp"
#include "TextButton.hpp"

#include <QLineEdit>
#include <QWidget>

class LibraryViewTopBar : public QWidget
{
	Q_OBJECT

public:
	explicit LibraryViewTopBar(QWidget *parent = nullptr);

signals:
	void searchRequested(const QString &query);
	void filterChanged(ViewFilter filter);
	void zoomRequested(int zoomValue);

private:
	TextButton *showAllButton;
	TextButton *showToWatchButton;

	IconButton *searchButton;
	IconButton *closeButton;
	IconButton *zoomInButton;
	IconButton *zoomOutButton;
	QLineEdit *searchInput;

	void setupLayout();
	void connectSignals();

	void onShowAllClicked();
	void onShowToWatchClicked();
	void onSearchCommitted();
	void onSearchTextChanged(const QString &text);

	void openSearch();
	void closeSearch();

	bool eventFilter(QObject *obj, QEvent *event) override;
};