#pragma once

#include "AppStorage.hpp"

#include <QDialog>
#include <QFrame>
#include <QLineEdit>
#include <QPushButton>

class SettingsWindow : public QDialog
{
	Q_OBJECT

  public:
	explicit SettingsWindow(AppStorage &appStorage, QWidget *parent = nullptr);

  private:
	AppStorage  &appStorage;
	QPushButton *lightTab;
	QPushButton *darkTab;
	QLineEdit   *apiKeyEdit;
	QPushButton *applyButton;
	QWidget     *platformsList = nullptr;
	QPushButton *addPlatformButton = nullptr;

	void setupUi();

	QWidget *makeThemeSection();
	QWidget *makeApiKeySection();
	QWidget *makeCustomStreamingPlatformsSection();
	QFrame  *makeSeparator();

	void    onApplyClicked();
	void    switchTheme(const QString &theme);
	void    refreshStyle();
	void    refreshPlatformsList();
	QString buildStyleSheet() const;

  signals:
	void themeChanged(const QString &theme);
};
