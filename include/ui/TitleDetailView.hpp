#pragma once

#include "AppStorage.hpp"
#include "IconButton.hpp"
#include "IconTextButton.hpp"
#include "Title.hpp"

#include <QLabel>
#include <QWidget>

class QFrame;
class QVBoxLayout;

class TitleDetailView : public QWidget
{
	Q_OBJECT

  public:
	explicit TitleDetailView(AppStorage &appStorage, QWidget *parent = nullptr);

	void setTitle(const Title &title);
	void refreshStyle();

  signals:
	void backRequested();

  protected:
	void resizeEvent(QResizeEvent *event) override;

  private:
	AppStorage &appStorage;
	Title       currentTitle;
	QPixmap     currentPoster;

	QWidget        *topBar;
	QLabel         *posterLabel;
	QWidget        *infoContainer;
	IconButton     *backBtn = nullptr;
	IconButton     *deleteBtn = nullptr;
	IconTextButton *toWatchBtn;
	IconTextButton *watchedBtn;

	QLabel  *watchedValueLabel = nullptr;
	QWidget *lastWatchedRow = nullptr;
	QLabel  *lastWatchedValueLabel = nullptr;

	void     setupUi();
	QWidget *buildTopBar();
	QWidget *buildContentRow();

	void populateInfo(const Title &title);
	void addHeaderSection(QVBoxLayout *layout, const Title &title);
	void addMetaSection(QVBoxLayout *layout, const Title &title);
	void addPlotSection(QVBoxLayout *layout, const Title &title);
	void addWatchOnSection(QVBoxLayout *layout, const Title &title);

	void updatePosterSize();
	void updateWatchButtons();
	void updateWatchState();
	void onWatchToggled();
	void onDeleteClicked();

	static QWidget *makeRow(const QString &label, const QString &value);
	static QFrame  *makeSeparator();
};
