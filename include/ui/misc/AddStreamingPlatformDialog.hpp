#pragma once

#include "AppStorage.hpp"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>

class AddStreamingPlatformDialog : public QDialog
{
	Q_OBJECT

  public:
	explicit AddStreamingPlatformDialog(
	    const QSet<QString> &existingNames, QWidget *parent = nullptr
	);

	StreamingPlatform platform() const;
	QString           imagePath() const;

  private:
	QSet<QString> existingNames;
	QLineEdit    *nameEdit;
	QLineEdit    *urlEdit;
	QLineEdit    *imagePathEdit;
	QPushButton  *addButton;
	QLabel       *errorLabel;

	void setupUi();
	void browseImage();
	void updateAddButton();
};
