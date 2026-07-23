#pragma once

class Viewport;
class ContentBrowser;
class GameObjectDetailsPanel;

struct EditorInfo {
	Viewport& viewport;
	ContentBrowser& contentBrowser;
	GameObjectDetailsPanel& detailsPanel;
};