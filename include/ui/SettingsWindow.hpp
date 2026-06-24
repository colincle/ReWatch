#pragma once

#include "AppStorage.hpp"
#include "StyledDialog.hpp"

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>

class SettingsWindow : public StyledDialog
{
	Q_OBJECT

  public:
	explicit SettingsWindow(AppStorage &appStorage, QWidget *parent = nullptr);

  private:
	AppStorage     &appStorage;
	QStackedWidget *sectionStack = nullptr;
	QPushButton    *sectionTabBtns[4] = {};
	QPushButton    *lightTab;
	QPushButton    *darkTab;
	QLineEdit      *apiKeyEdit;
	QPushButton    *applyButton;
	QSpinBox       *maxUpdateRequestsSpinBox;
	QWidget        *platformsList = nullptr;
	QPushButton    *addPlatformButton = nullptr;
	QWidget        *darkAccentRow = nullptr;
	QPushButton    *darkColorSwatch = nullptr;
	QPushButton    *resetDarkAccentButton = nullptr;
	QWidget        *lightAccentRow = nullptr;
	QPushButton    *lightColorSwatch = nullptr;
	QPushButton    *resetLightAccentButton = nullptr;

	void setupUi();

	QLabel  *makeSectionTitle(const QString &text);
	QWidget *makeAccentRow(bool isDark, QPushButton *&swatch, QPushButton *&reset);
	QWidget *makeAppearanceSection();
	QWidget *makeApiKeySection();
	QWidget *makeCustomStreamingPlatformsSection();
	QWidget *makeRankingSection();
	QFrame  *makeSeparator();

	void    switchSection(int index);
	void    onApplyClicked();
	void    onAddPlatformClicked();
	void    confirmResetRankings(const QString &label, const QString &type);
	void    switchTheme(const QString &theme);
	void    refreshStyle();
	void    refreshPlatformsList();
	QString buildStyleSheet() const;
};
