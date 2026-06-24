#include "HelpDialog.hpp"
#include "HelpItems.hpp"
#include "Palette.hpp"

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>

HelpDialog::HelpDialog(QWidget *parent) : QDialog(parent)
{
	setWindowTitle("How to use ReWatch");
	resize(520, 580);
	setMinimumSize(380, 320);
	setStyleSheet(QStringLiteral("background-color: %1;").arg(Palette::bgPrimary));

	searchInput = new QLineEdit(this);
	searchInput->setPlaceholderText("Search...");
	searchInput->setStyleSheet(
	    QStringLiteral(
	        "QLineEdit {"
	        "    background-color: %1; color: %2; border: 1px solid %3;"
	        "    border-radius: 8px; padding: 6px 12px; font-size: 13px;"
	        "}"
	        "QLineEdit:focus { border-color: %4; }"
	    )
	        .arg(
	            Palette::bgSecondary,
	            Palette::textPrimary,
	            Palette::border,
	            Palette::accent
	        )
	);

	listContainer = new QWidget;
	listLayout = new QVBoxLayout(listContainer);
	listLayout->setContentsMargins(0, 0, 0, 0);
	listLayout->setSpacing(8);

	buildItems();
	listLayout->addStretch();

	auto *scroll = new QScrollArea(this);
	scroll->setWidget(listContainer);
	scroll->setWidgetResizable(true);
	scroll->setFrameShape(QFrame::NoFrame);
	scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll->setStyleSheet(
	    QStringLiteral("background-color: %1;").arg(Palette::bgPrimary)
	);

	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(20, 20, 20, 20);
	root->setSpacing(12);
	root->addWidget(searchInput);
	root->addWidget(scroll);

	connect(searchInput, &QLineEdit::textChanged, this, &HelpDialog::onSearchChanged);
}

void HelpDialog::buildItems()
{
	for(const HelpItem &item : helpItems())
	{
		auto *card = new QWidget(listContainer);
		card->setStyleSheet(
		    QStringLiteral("QWidget { background-color: %1; border-radius: 8px; }")
		        .arg(Palette::bgSecondary)
		);

		auto *cardLayout = new QVBoxLayout(card);
		cardLayout->setContentsMargins(16, 12, 16, 12);
		cardLayout->setSpacing(6);

		auto *nameLabel = new QLabel(item.name, card);
		QFont nameFont;
		nameFont.setPixelSize(13);
		nameFont.setBold(true);
		nameLabel->setFont(nameFont);
		nameLabel->setStyleSheet(QStringLiteral("color: %1; background: transparent;")
		                             .arg(Palette::textPrimary));

		auto *descLabel = new QLabel(item.description, card);
		descLabel->setWordWrap(true);
		descLabel->setStyleSheet(
		    QStringLiteral(
		        "color: %1; background: transparent; font-size: 12px; line-height: 1.4;"
		    )
		        .arg(Palette::textSecondary)
		);

		cardLayout->addWidget(nameLabel);
		cardLayout->addWidget(descLabel);

		listLayout->addWidget(card);

		items.push_back({card, item.name + " " + item.description});
	}
}

void HelpDialog::onSearchChanged(const QString &query)
{
	const QString q = query.trimmed();
	for(const ItemWidget &iw : items)
		iw.widget->setVisible(
		    q.isEmpty() || iw.searchText.contains(q, Qt::CaseInsensitive)
		);
}
