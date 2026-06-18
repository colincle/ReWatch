#pragma once

#include "AppStorage.hpp"
#include "ElidedLabel.hpp"
#include "IconButton.hpp"
#include "IconTextButton.hpp"
#include "Title.hpp"

#include <QWidget>

class TitleCard : public QWidget
{
	Q_OBJECT

  public:
	explicit TitleCard(
	    const Title &title, AppStorage &appStorage, int cardWidth,
	    QWidget *parent = nullptr
	);

  protected:
	void enterEvent(QEnterEvent *event) override;
	void leaveEvent(QEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

  signals:
	void clicked();

  private:
	Title       title;
	AppStorage &appStorage;
	int         cardWidth;
	int         posterHeight;
	int         titleLabelHeight;

	QPixmap         posterPixmap;
	ElidedLabel    *titleLabel;
	IconTextButton *notViewedButton;
	IconTextButton *viewedButton;
	IconButton     *deleteButton;
	IconButton     *uploadPosterButton;

	void setupUi();
	void setupTitleLabel();
	void setupButtons();
	void connectButtons();
	void showButtons();
	void hideButtons();

	void onViewedClicked();
	void onNotViewedClicked();
	void onDeleteClicked();
	void onUploadPosterClicked();
};