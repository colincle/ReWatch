#include "SearchResults.hpp"
#include "AssetsPaths.hpp"
#include "ColorPalette.hpp"
#include "IconButton.hpp"
#include "OmdbSearch.hpp"
#include "Spinner.hpp"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>

SearchResults::SearchResults(AppStorage &storage, QWidget *parent)
    : QWidget(parent)
    , appStorage(storage)
{
    setStyleSheet("background-color: " COLOR_BG_PRIMARY ";");
    setAttribute(Qt::WA_StyledBackground, true);

    setupLayout();
}

void SearchResults::setupLayout()
{
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);

    spinner = new Spinner(COLOR_ACCENT, 8, this);
    spinner->setFixedSize(48, 48);
    spinner->hide();

    layout->addWidget(spinner, 0, Qt::AlignCenter);

    resultsContainer = new QWidget;
    resultsLayout    = new QVBoxLayout(resultsContainer);
    resultsLayout->setContentsMargins(0, 0, 0, 0);
    resultsLayout->setSpacing(12);

    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setWidget(resultsContainer);
    scrollArea->hide();

    layout->addWidget(scrollArea);
}

void SearchResults::search(QString query)
{
    spinner->show();
    scrollArea->hide();
    clearExtraLayoutWidgets();
    scrollArea->verticalScrollBar()->setValue(0);
    clearResultsLayout();

    auto *omdbSearch = new OmdbSearch(appStorage, query, appStorage.getKey(), this);

    connect(omdbSearch, &OmdbSearch::searchFinished, this, [this, omdbSearch]() {
        spinner->hide();
        onSearchFinished(omdbSearch);
    });

    omdbSearch->search();
}

void SearchResults::onSearchFinished(OmdbSearch *omdbSearch)
{
    const results &r = omdbSearch->getResults();

    if (!r.error.isEmpty())
    {
        setFullPageState(r.error == "Host requires authentication"
            ? API_KEY_ERROR
            : NO_MOVIES_FOUND);

        omdbSearch->deleteLater();
        return;
    }

    for (const resultTitle &title : r.titles)
        resultsLayout->addWidget(makeResultRow(title));

    resultsLayout->addStretch();
    scrollArea->show();
    omdbSearch->deleteLater();
}

QWidget *SearchResults::makeResultRow(const resultTitle &title)
{
    auto *row = new QWidget;
    row->setStyleSheet(
        "background-color: " COLOR_BG_SECONDARY ";"
        "border: 1px solid " COLOR_BORDER ";"
        "border-radius: 10px;"
    );

    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(12, 12, 12, 12);
    rowLayout->setSpacing(20);

    rowLayout->addWidget(makePosterLabel(title));
    rowLayout->addWidget(makeTitleLabel(title));
    rowLayout->addStretch();
    rowLayout->addWidget(appStorage.contains(title.imdbId)
        ? makeDoneButton(title, row)
        : makeAddButton(title, row));

    return row;
}

QLabel *SearchResults::makePosterLabel(const resultTitle &title)
{
    auto *poster = new QLabel;
    poster->setFixedSize(100, 150);
    poster->setStyleSheet("border: none; background: transparent;");
    poster->setPixmap(title.posterImage.scaled(
        poster->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation));
    return poster;
}

QLabel *SearchResults::makeTitleLabel(const resultTitle &title)
{
    auto *label = new QLabel(title.title);
    label->setStyleSheet(
        "color: " COLOR_TEXT_PRIMARY ";"
        "font-size: 24px;"
        "font-weight: bold;"
        "border: none;"
        "background: transparent;"
    );
    label->setWordWrap(true);
    return label;
}

IconButton *SearchResults::makeDoneButton(const resultTitle &title, QWidget *row)
{
    auto *doneButton = new IconButton(ADDED_ICON, 40, COLOR_SUCCESS, COLOR_SURFACE, row);

    connect(doneButton, &QPushButton::clicked, this, [this, title, doneButton, row]() {
        appStorage.deleteTitle(title.imdbId);
        auto *addButton = makeAddButton(title, row);
        qobject_cast<QHBoxLayout *>(row->layout())->replaceWidget(doneButton, addButton);
        doneButton->deleteLater();
    });

    return doneButton;
}

IconButton *SearchResults::makeAddButton(const resultTitle &title, QWidget *row)
{
    auto *addButton = new IconButton(ADD_ICON, 40, COLOR_ACCENT, COLOR_SURFACE, row);

    connect(addButton, &QPushButton::clicked, this, [this, title, addButton, row]() {
        onAddClicked(title, addButton, row);
    });

    return addButton;
}

void SearchResults::onAddClicked(const resultTitle &title, IconButton *addButton, QWidget *row)
{
    auto *rowSpinner = new Spinner(COLOR_ACCENT, 6, row);
    rowSpinner->setFixedSize(40, 40);

    auto *rowLayout = qobject_cast<QHBoxLayout *>(row->layout());
    rowLayout->replaceWidget(addButton, rowSpinner);
    addButton->hide();

    auto *fetch = new OmdbSearch(appStorage, "", appStorage.getKey(), this);

    connect(fetch, &OmdbSearch::titleFetched, this,
        [this, title, rowSpinner, addButton, row, fetch]() {
            auto *doneButton = makeDoneButton(title, row);
            qobject_cast<QHBoxLayout *>(row->layout())->replaceWidget(rowSpinner, doneButton);
            rowSpinner->deleteLater();
            addButton->deleteLater();
            fetch->deleteLater();
        });

    fetch->fetchById(title.imdbId, title.posterImage);
}

void SearchResults::setFullPageState(const QString &imagePath)
{
    clearResultsLayout();
    scrollArea->hide();
    clearExtraLayoutWidgets();

    auto *label = new QLabel(this);
    label->setPixmap(QPixmap(imagePath));
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    label->setMinimumSize(1, 1);
    label->setScaledContents(false);

    layout->addWidget(label, 1);
}

void SearchResults::clearResultsLayout()
{
    while (QLayoutItem *item = resultsLayout->takeAt(0))
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }
}

void SearchResults::clearExtraLayoutWidgets()
{
    for (int i = layout->count() - 1; i >= 0; --i)
    {
        QLayoutItem *item = layout->itemAt(i);
        if (item->widget() &&
            item->widget() != spinner &&
            item->widget() != scrollArea)
        {
            item->widget()->deleteLater();
            layout->removeItem(item);
        }
    }
}