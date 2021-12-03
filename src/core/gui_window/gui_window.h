#pragma once

#include <string>

#include "utils/gui_utils.h"

struct GuiWindowProperties {
    public:
	GuiWindowProperties();

    public:
	std::string name;
	ImVec2 pos;
	ImVec2 size;
	float alpha;
	bool active;

	/* For child windows. */
	bool border;

	bool hasTitle;
	bool hasResize;
	bool hasMove;
	bool hasScrollBar;
	bool hasCollapse;
	bool hasAutoResize;
	bool hasBackground;
	bool hasMouseInputs;
	bool hasMenuBar;
	bool hasHorizontalScrollbar;
	bool hasFocusOnAppearing;
	bool hasBringToFrontOnFocus;

	bool alwaysShowVerticalScrollbar;
	bool alwaysShowHorizontalScrollbar;
	bool alwaysUseWindowPadding;

	bool hasUnsavedDot;
	bool hasDocking;
	bool hasNavFocus;
};

class GuiWindow {
    public:
	GuiWindow();
	GuiWindow(GuiWindowProperties props);
	~GuiWindow();

    private:
    private:
};
