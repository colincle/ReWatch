#include "AddStreamingPlatformDialog.hpp"
#include "AssetsPaths.hpp"
#include "IconButton.hpp"
#include "Palette.hpp"
#include "SvgUtils.hpp"

#include <QColor>
#include <QClipboard>
#include <QFileDialog>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

AddStreamingPlatformDialog::AddStreamingPlatformDialog(
    const QSet<QString> &existingNames, QWidget *parent
)
    : QDialog(parent), existingNames(existingNames)
{
	setWindowTitle("Add streaming platform");
	setModal(true);
	setupUi();
}

void AddStreamingPlatformDialog::setupUi()
{
	setStyleSheet(
	    QStringLiteral(
	        "QDialog { background-color: %1; }"
	        "QLabel { color: %2; background: transparent; }"
	        "QLineEdit { background-color: %3; color: %2; border: 1px solid %4; "
	        "border-radius: 6px; padding: 6px 10px; }"
	        "QPushButton { background-color: %3; color: %5; border: none; "
	        "border-radius: 6px; padding: 6px 18px; }"
	        "QPushButton:pressed { background-color: %7; color: %5; }"
	        "QPushButton:disabled { background-color: %3; color: %6; }"
	    )
	        .arg(
	            Palette::bgPrimary,
	            Palette::textPrimary,
	            Palette::surface,
	            Palette::border,
	            Palette::accent,
	            Palette::textSecondary,
	            QColor(Palette::surface).darker(115).name()
	        )
	);

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(24, 24, 24, 24);
	layout->setSpacing(16);
	layout->setSizeConstraint(QLayout::SetFixedSize);

	auto makeField =
	    [&](const QString &label, QLineEdit *&edit, const QString &placeholder)
	{
		auto *fieldLayout = new QVBoxLayout;
		fieldLayout->setSpacing(6);
		auto *lbl = new QLabel(label);
		edit = new QLineEdit;
		edit->setPlaceholderText(placeholder);
		fieldLayout->addWidget(lbl);
		fieldLayout->addWidget(edit);
		layout->addLayout(fieldLayout);
	};

	makeField("Name", nameEdit, "My streaming service...");
	nameEdit->setMaxLength(20);

	errorLabel = new QLabel("A platform with this name already exists");
	errorLabel->setStyleSheet(QString("color: %1;").arg(Palette::error));
	errorLabel->setVisible(false);
	layout->addWidget(errorLabel);

	makeField("Search URL", urlEdit, "");

	auto *hint = new QLabel(
	    "Go to your streaming platform, search for exactly \"movietracker\",\n"
	    "then copy the URL from your address bar and paste it here.\n\n"
	    "This may not work with every platform. If it doesn't,\n"
	    "delete it and add it again with just the homepage URL the\n"
	    "button will take you there.\n\n"
	    "Platform updates may also break it -- if that happens,\n"
	    "delete it and recreate it."
	);
	hint->setWordWrap(true);
	hint->setStyleSheet(QString("color: %1;").arg(Palette::textSecondary));

	auto *copyBtn = new QPushButton("Copy \"movietracker\"");
	copyBtn->setAutoDefault(false);
	copyBtn->setIcon(loadColoredSvg(AssetsPaths::copyIcon, Palette::accent, 16));
	copyBtn->ensurePolished();
	copyBtn->setFixedWidth(copyBtn->sizeHint().width());
	connect(
	    copyBtn,
	    &QPushButton::clicked,
	    this,
	    [copyBtn]()
	    {
		    QGuiApplication::clipboard()->setText("movietracker");
		    copyBtn->setText("Copied!");
		    copyBtn->setEnabled(false);
		    QTimer::singleShot(
		        1500,
		        copyBtn,
		        [copyBtn]()
		        {
			        copyBtn->setText("Copy \"movietracker\"");
			        copyBtn->setEnabled(true);
		        }
		    );
	    }
	);

	auto *hintRow = new QHBoxLayout;
	hintRow->setContentsMargins(0, 0, 0, 0);
	hintRow->addWidget(hint, 1);
	hintRow->addWidget(copyBtn, 0, Qt::AlignBottom);
	layout->addLayout(hintRow);

	auto *imageLayout = new QVBoxLayout;
	imageLayout->setSpacing(6);
	imageLayout->addWidget(new QLabel("Logo (optional)"));

	auto *imageRow = new QHBoxLayout;
	imagePathEdit = new QLineEdit;
	imagePathEdit->setPlaceholderText("Path to logo image...");
	imagePathEdit->setReadOnly(true);

	auto *browseBtn = new IconButton(
	    AssetsPaths::imageUploadIcon,
	    36,
	    Palette::accent,
	    Palette::surface,
	    this
	);
	browseBtn->setAutoDefault(false);

	imageRow->addWidget(imagePathEdit, 1);
	imageRow->addWidget(browseBtn);
	imageLayout->addLayout(imageRow);
	layout->addLayout(imageLayout);

	auto *buttonRow = new QHBoxLayout;
	buttonRow->addStretch();

	auto *cancelButton = new QPushButton("Cancel");
	cancelButton->setAutoDefault(false);
	addButton = new QPushButton("Add");
	addButton->setAutoDefault(false);
	addButton->setEnabled(false);

	buttonRow->addWidget(cancelButton);
	buttonRow->addWidget(addButton);
	layout->addLayout(buttonRow);

	connect(
	    nameEdit,
	    &QLineEdit::textChanged,
	    this,
	    &AddStreamingPlatformDialog::updateAddButton
	);
	connect(
	    urlEdit,
	    &QLineEdit::textChanged,
	    this,
	    &AddStreamingPlatformDialog::updateAddButton
	);
	connect(
	    browseBtn,
	    &QPushButton::clicked,
	    this,
	    &AddStreamingPlatformDialog::browseImage
	);
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
	connect(addButton, &QPushButton::clicked, this, &QDialog::accept);
}

void AddStreamingPlatformDialog::browseImage()
{
	const QString path = QFileDialog::getOpenFileName(
	    this,
	    "Select logo",
	    {},
	    "Images (*.png *.jpg *.jpeg *.svg *.webp)"
	);

	if(!path.isEmpty())
	{
		imagePathEdit->setText(path);
	}
}

void AddStreamingPlatformDialog::updateAddButton()
{
	const QString name = nameEdit->text().simplified();
	const bool    duplicate = existingNames.contains(name);
	errorLabel->setVisible(duplicate);
	addButton->setEnabled(
	    !name.isEmpty() && !urlEdit->text().simplified().isEmpty() && !duplicate
	);
}

StreamingPlatform AddStreamingPlatformDialog::platform() const
{
	return {urlEdit->text().simplified(), nameEdit->text().simplified(), {}};
}

QString AddStreamingPlatformDialog::imagePath() const
{
	return imagePathEdit->text();
}
