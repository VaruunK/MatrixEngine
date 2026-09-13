#pragma once

class Viewport;
class ContentBrowser;
class DetailsPanel;
class OutlinerPanel;

struct EditorInfo {
	Viewport& viewport;
	ContentBrowser& contentBrowser;
	DetailsPanel& detailsPanel;
	OutlinerPanel& outlinerPanel;
};