// Base class for all dialogs. Applies a consistent stylesheet on construction;
// subclasses call applyStyleSheet() again after theme changes.
#pragma once

#include "Palette.hpp"

#include <QColor>
#include <QDialog>

class StyledDialog : public QDialog
{
  public:
	explicit StyledDialog(QWidget *parent = nullptr) : QDialog(parent)
	{
		applyStyleSheet();
	}

	void applyStyleSheet()
	{
		setStyleSheet(
		    QStringLiteral(
		        "QDialog { background-color: %1; }"
		        "QLabel { color: %2; background: transparent; }"
		        "QLineEdit { background-color: %3; color: %2; border: 1px solid %4; "
		        "border-radius: 6px; padding: 6px 10px; }"
		        "QPushButton { background-color: %3; color: %5; border: none; "
		        "border-radius: 6px; padding: 6px 18px; }"
		        "QPushButton:pressed { background-color: %6; color: %5; }"
		        "QPushButton:disabled { background-color: %3; color: %7; }"
		    )
		        .arg(
		            Palette::bgPrimary,
		            Palette::textPrimary,
		            Palette::surface,
		            Palette::border,
		            Palette::accent,
		            QColor(Palette::surface).darker(115).name(),
		            Palette::textSecondary
		        )
		);
	}
};
