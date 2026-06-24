// Modal dialog for adding a custom streaming platform (name, search URL, optional logo).
#pragma once

#include "AppStorage.hpp"
#include "StyledDialog.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>

class AddStreamingPlatformDialog : public StyledDialog
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
