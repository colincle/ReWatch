#pragma once

#include "AppStorage.hpp"
#include "Spinner.hpp"
#include "OmdbSearch.hpp"
#include "IconButton.hpp"

#include <QWidget>
#include <QVBoxLayout>
#include <QString>
#include <QScrollArea>
#include <QLabel>

class SearchResults : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResults(AppStorage &storage, QWidget *parent = nullptr);
    void search(QString query);

private:
    void        setupLayout();
    void        onSearchFinished(OmdbSearch *omdbSearch);
    void        onAddClicked(const resultTitle &title, IconButton *addButton, QWidget *row);
    QWidget*    makeResultRow(const resultTitle &title);
    QLabel*     makePosterLabel(const resultTitle &title);
    QLabel*     makeTitleLabel(const resultTitle &title);
    IconButton* makeDoneButton(const resultTitle &title, QWidget *row);
    IconButton* makeAddButton(const resultTitle &title, QWidget *row);
    void        setFullPageState(const QString &imagePath);
    void        clearResultsLayout();
    void        clearExtraLayoutWidgets();

    AppStorage  &appStorage;
    QVBoxLayout *layout;
    Spinner     *spinner;
    QWidget     *resultsContainer;
    QVBoxLayout *resultsLayout;
    QScrollArea *scrollArea;
};