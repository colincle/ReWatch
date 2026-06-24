// Native macOS menu bar with Library > Import/Export actions.
#pragma once

#include "AppStorage.hpp"

#include <QMenuBar>

class AppMenuBar : public QMenuBar
{
	Q_OBJECT

  public:
	explicit AppMenuBar(AppStorage &appStorage, QWidget *parent = nullptr);

  private:
	AppStorage &appStorage;

	void onImportLibraryTriggered();
	void onExportLibraryTriggered();
};
