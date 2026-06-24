// Button for a streaming platform entry on the title detail page, with logo and name.
#pragma once

#include "AppStorage.hpp"
#include "HoverButton.hpp"

class StreamingPlatformButton : public HoverButton
{
  public:
	explicit StreamingPlatformButton(
	    const StreamingPlatform &platform, int height, QString color1, QString color2,
	    QWidget *parent = nullptr
	);

  protected:
	void applyNormal() override;
	void applyHover() override;

  private:
	QString color1;
	QString color2;
	int     iconPadding;
};
