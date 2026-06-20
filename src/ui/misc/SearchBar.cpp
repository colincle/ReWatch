#include "SearchBar.hpp"
#include "Palette.hpp"

#include <QKeyEvent>

static constexpr int HEIGHT = 40;

SearchBar::SearchBar(QWidget *parent) : QLineEdit(parent)
{
	setPlaceholderText(" Search...");
	setClearButtonEnabled(true);
	setFixedHeight(HEIGHT);
	setFrame(false);
	setStyleSheet(QStringLiteral(
	                  "QLineEdit {"
	                  "    background-color: %1;"
	                  "    color: %2;"
	                  "    border: 1px solid %3;"
	                  "    border-radius: 10px;"
	                  "    padding-left: 12px;"
	                  "    padding-right: 28px;"
	                  "    selection-background-color: %4;"
	                  "    selection-color: white;"
	                  "}"
	)
	                  .arg(
	                      Palette::surface,
	                      Palette::textSecondary,
	                      Palette::border,
	                      Palette::accentLight
	                  ));
}

void SearchBar::refreshStyle()
{
	setStyleSheet(QStringLiteral(
	                  "QLineEdit {"
	                  "    background-color: %1;"
	                  "    color: %2;"
	                  "    border: 1px solid %3;"
	                  "    border-radius: 10px;"
	                  "    padding-left: 12px;"
	                  "    padding-right: 28px;"
	                  "    selection-background-color: %4;"
	                  "    selection-color: white;"
	                  "}"
	)
	                  .arg(
	                      Palette::surface,
	                      Palette::textSecondary,
	                      Palette::border,
	                      Palette::accentLight
	                  ));
}

void SearchBar::keyPressEvent(QKeyEvent *event)
{
	if(event->key() == Qt::Key_Escape)
	{
		emit escapePressed();
		return;
	}

	QLineEdit::keyPressEvent(event);
}
