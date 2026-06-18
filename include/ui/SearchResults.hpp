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
	void onAddClicked(const ResultTitle &title, IconButton *addButton, QWidget *row);
	void restoreRowButton(
	    QWidget *row, Spinner *rowSpinner, IconButton *oldAddButton,
	    IconButton *replacement
	);
	QWidget    *makeResultRow(const ResultTitle &title);
	QLabel     *makePosterLabel(const ResultTitle &title);
	QWidget    *makeTitleInfo(const ResultTitle &title);
	QLabel     *makeTitleLabel(const ResultTitle &title);
	QLabel     *makeYearLabel(const ResultTitle &title);
	QLabel     *makePlotLabel(const ResultTitle &title);
	IconButton *makeDoneButton(const ResultTitle &title, QWidget *row);
	IconButton *makeAddButton(const ResultTitle &title, QWidget *row);
	void        setFullPageState(const QString &imagePath);
	void        clearResultsLayout();
	void        clearExtraLayoutWidgets();

	AppStorage  &appStorage;
	QVBoxLayout *layout;
	Spinner     *spinner;
	QWidget     *resultsContainer;
	QGridLayout *resultsLayout;
	QScrollArea *scrollArea;
	OmdbSearch  *currentSearch = nullptr;
};
