#pragma once

#include "AppStorage.hpp"
#include "IconButton.hpp"
#include "OmdbSearch.hpp"
#include "Spinner.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

class SearchResults : public QWidget
{
	Q_OBJECT

public:
	explicit SearchResults(AppStorage &storage, QWidget *parent = nullptr);
	void search(QString query);

signals:
	void searchError(const QString &message);

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	void setupLayout();
	void onSearchFinished(OmdbSearch *omdbSearch);
	void onAddClicked(const resultTitle &title, IconButton *addButton, QWidget *row);
	void restoreRowButton(QWidget *row, Spinner *rowSpinner, IconButton *oldAddButton, IconButton *replacement);
	QWidget *makeResultRow(const resultTitle &title);
	QLabel *makePosterLabel(const resultTitle &title);
	QWidget *makeTitleInfo(const resultTitle &title);
	QLabel *makeTitleLabel(const resultTitle &title);
	QLabel *makeYearLabel(const resultTitle &title);
	QLabel *makePlotLabel(const resultTitle &title);
	IconButton *makeDoneButton(const resultTitle &title, QWidget *row);
	IconButton *makeAddButton(const resultTitle &title, QWidget *row);
	void setFullPageState(const QString &imagePath);
	void clearResultsLayout();
	void clearExtraLayoutWidgets();

	AppStorage &appStorage;
	QVBoxLayout *layout;
	Spinner *spinner;
	QWidget *resultsContainer;
	QGridLayout *resultsLayout;
	QScrollArea *scrollArea;
	OmdbSearch *currentSearch = nullptr;
};
