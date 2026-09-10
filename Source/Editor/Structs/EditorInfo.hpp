#pragma once

class Viewport;
class ContentBrowser;
class DetailsPanel;

struct EditorInfo {
	Viewport& viewport;
	ContentBrowser& contentBrowser;
	DetailsPanel& detailsPanel;
};