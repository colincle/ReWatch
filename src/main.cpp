#include "planner.hpp"
#include "globals.hpp"

#include "leftPanel.hpp"
#include "rightPanel.hpp"
#include "menuButton.hpp"

#include <cstdlib>
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>
#include <QtCore/QObject>
#include <QShortcut>
#include <QStyleFactory>

allSeries g_allSeries;
planner g_planner;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QWidget window;
	window.setAttribute(Qt::WA_StyledBackground, true);
	window.setAutoFillBackground(true);
	window.setStyleSheet("background-color: rgb(0, 0, 0);");
	window.setWindowTitle("Manga Planner");

	QShortcut* closeShortcut = new QShortcut(QKeySequence("Ctrl+W"), &window);
	QObject::connect(closeShortcut, &QShortcut::activated, &window, &QWidget::close);

	QHBoxLayout* mainLayout = new QHBoxLayout(&window);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	LeftPanel* leftPanel = new LeftPanel(&window);
	RightPanel* rightPanel = new RightPanel(&window);

	QObject::connect(leftPanel, &LeftPanel::pageSelected, rightPanel, &RightPanel::setStatus);

	mainLayout->addWidget(leftPanel, 3);
	mainLayout->addWidget(rightPanel, 7);

	window.resize(1280, 720);
	window.setMinimumSize(840, 420);
	window.show();
	return app.exec();
}
