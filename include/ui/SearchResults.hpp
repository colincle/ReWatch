// Displays OMDb search results in a two-column grid. Manages OmdbSearch lifecycle —
// cancels the previous search when a new query arrives.
#pragma once

#include "AppStorage.hpp"
#include "IconButton.hpp"
#include "OmdbSearch.hpp"
#include "Spinner.hpp"

#include <vector>
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
	void refreshStyle();

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
	QWidget    *makeTitleInfo(const ResultTitle &title);
	IconButton *makeDoneButton(const ResultTitle &title, QWidget *row);
	IconButton *makeAddButton(const ResultTitle &title, QWidget *row);
	void        setFullPageState(const QString &imagePath);
	void        clearResultsLayout();
	void        clearExtraLayoutWidgets();

	AppStorage  &appStorage;
	QVBoxLayout *m_layout;
	Spinner     *spinner;
	QWidget     *resultsContainer;
	QGridLayout *resultsLayout;
	QScrollArea *scrollArea;
	OmdbSearch  *currentSearch = nullptr;

	std::vector<ResultTitle> lastResults;
	void                     rebuildResults();
};
