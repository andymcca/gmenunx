#include "browsedialog.h"
#include "messagebox.h"
#include "debug.h"
#include "utilities.h"
#include "powermanager.h"
extern const char *CARD_ROOT;

BrowseDialog::BrowseDialog(GMenu2X *gmenu2x, const string &title, const string &description, const string &icon):
Dialog(gmenu2x, title, description, icon) {
	srand(SDL_GetTicks());
}

bool BrowseDialog::exec(string _path) {
	this->bg = new Surface(gmenu2x->bg); // needed to redraw on child screen return

	Surface *iconGoUp = gmenu2x->sc["skin:imgs/go-up.png"];
	Surface *iconFolder = gmenu2x->sc["skin:imgs/folder.png"];
	Surface *iconFile = gmenu2x->sc["skin:imgs/file.png"];
	Surface *iconSd = gmenu2x->sc["skin:imgs/sd.png"];
	Surface *iconFav = gmenu2x->sc["skin:imgs/fav.png"];
	Surface *iconCur;

	uint32_t i, iY, firstElement = 0, padding = 6;
	int32_t animation = 0;
	uint32_t rowHeight = gmenu2x->font->height() + 1;
	uint32_t numRows = (gmenu2x->listRect.h - 2) / rowHeight - 1;

	if (_path.empty() || !dir_exists(_path))
		_path = gmenu2x->confStr["homePath"];

	directoryEnter(_path);

	string preview = getPreview(selected);

	// this->description = getFilter();

	while (true) {
		if (selected < 0) selected = this->size() - 1;
		if (selected >= this->size()) selected = 0;

		bool inputAction = false;

		buttons.clear();
		buttons.push_back({"select", _("Menu")});
		buttons.push_back({"a", _("Cancel")});

		if (!showFiles && allowSelectDirectory)
			buttons.push_back({"start", _("Select")});
		else if ((allowEnterDirectory && isDirectory(selected)) || !isDirectory(selected))
			buttons.push_back({"b", _("Select")});

		if (showDirectories && allowDirUp && path != "/")
			buttons.push_back({"x", _("Dir up")});

		if (gmenu2x->confStr["previewMode"] == "Backdrop") {
			if (!(preview.empty() || preview == "#"))
				gmenu2x->setBackground(this->bg, preview);
			else
				gmenu2x->bg->blit(this->bg,0,0);
		}

		this->description = path;

		drawDialog(gmenu2x->s);

		if (!size()) {
			MessageBox mb(gmenu2x, _("This directory is empty"));
			mb.setAutoHide(1);
			mb.setBgAlpha(0);
			mb.exec();
		} else {
			// Selection
			if (selected >= firstElement + numRows) firstElement = selected - numRows;
			if (selected < firstElement) firstElement = selected;

			// Files & Directories
			iY = gmenu2x->listRect.y + 1;
			for (i = firstElement; i < size() && i <= firstElement + numRows; i++, iY += rowHeight) {
				if (i == selected) gmenu2x->s->box(gmenu2x->listRect.x, iY, gmenu2x->listRect.w, rowHeight, gmenu2x->skinConfColor["selectionBg"]);

				iconCur = iconFile;

				if (isDirectory(i)) {
					if (getFile(i) == "..")
						iconCur = iconGoUp;
					else if (getPath(i) == "/media" || path == "/media")
						iconCur = iconSd;
					else
						iconCur = iconFolder;
				} else if (isFavourite(getFile(i))) {
					iconCur = iconFav;
				}

				iconCur->blit(gmenu2x->s, gmenu2x->listRect.x + 10, iY + rowHeight/2, HAlignCenter | VAlignMiddle);

				gmenu2x->s->write(gmenu2x->font, getFileName(i), gmenu2x->listRect.x + 21, iY + rowHeight/2, VAlignMiddle);
			}

			if (gmenu2x->confStr["previewMode"] != "Backdrop") {
				Surface anim = new Surface(gmenu2x->s);
				if (preview.empty() || preview == "#") { // hide preview
					 while (animation > 0) {
						animation -= gmenu2x->skinConfInt["previewWidth"] / 8;

						if (animation < 0)
							animation = 0;

						anim.blit(gmenu2x->s,0,0);
						gmenu2x->s->box(gmenu2x->platform->w - animation, gmenu2x->listRect.y, gmenu2x->skinConfInt["previewWidth"] + 2 * padding, gmenu2x->listRect.h, gmenu2x->skinConfColor["previewBg"]);
						gmenu2x->s->flip();
						SDL_Delay(10);
					};
				} else { // show preview
					if (!gmenu2x->sc.exists(preview + "scaled")) {
						Surface *previm = new Surface(preview);
						gmenu2x->sc.add(previm, preview + "scaled");

						gmenu2x->sc[preview + "scaled"]->softStretch(gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h - 2 * padding, SScaleFit);
					}

					do {
						animation += gmenu2x->skinConfInt["previewWidth"] / 8;

						if (animation > gmenu2x->skinConfInt["previewWidth"] + 2 * padding)
							animation = gmenu2x->skinConfInt["previewWidth"] + 2 * padding;

						anim.blit(gmenu2x->s,0,0);
						gmenu2x->s->box(gmenu2x->platform->w - animation, gmenu2x->listRect.y, gmenu2x->skinConfInt["previewWidth"] + 2 * padding, gmenu2x->listRect.h, gmenu2x->skinConfColor["previewBg"]);
						gmenu2x->sc[preview + "scaled"]->blit(gmenu2x->s, {gmenu2x->platform->w - animation + padding, gmenu2x->listRect.y + padding, gmenu2x->skinConfInt["previewWidth"], gmenu2x->listRect.h - 2 * padding}, HAlignCenter | VAlignMiddle, gmenu2x->platform->h);
						gmenu2x->s->flip();
						SDL_Delay(10);
					} while (animation < gmenu2x->skinConfInt["previewWidth"] + 2 * padding);
				}
			}
			gmenu2x->drawScrollBar(numRows, size(), firstElement, gmenu2x->listRect);
			gmenu2x->s->flip();
		}

		do {
			inputAction = gmenu2x->input.update();
			if (gmenu2x->inputCommonActions(inputAction)) continue;

			if (gmenu2x->input[UP]) {
				selected--;
			} else if (gmenu2x->input[DOWN]) {
				selected++;
			} else if (gmenu2x->input[LEFT]) {
				selected -= numRows;
				if (selected < 0) selected = 0;
			} else if (gmenu2x->input[RIGHT]) {
				selected += numRows;
				if (selected >= this->size()) selected = this->size() - 1;
			} else if (gmenu2x->input[PAGEDOWN]) {
				string cur = getFileName(selected);
				while ((selected < this->size() - 1) && selected++) {
					string sel = getFileName(selected);
					if (tolower(cur.at(0)) != tolower(sel.at(0)))
						break;
				}
			} else if (gmenu2x->input[PAGEUP]) {
				string cur = getFileName(selected);
				while (selected > 0 && selected--) {
					string sel = getFileName(selected);
					if (tolower(cur.at(0)) != tolower(sel.at(0)))
						break;
				}
			} else if (showDirectories && allowDirUp && (gmenu2x->input[MODIFIER] || (gmenu2x->input[CONFIRM] && getFile(selected) == ".."))) { /*Directory Up */
				selected = 0;
				preview = "";
				if (browse_history.size() > 0) {
					selected = browse_history.back();
					browse_history.pop_back();
				}
				directoryEnter(path + "/..");
			} else if (gmenu2x->input[CONFIRM]) {
				if (allowEnterDirectory && isDirectory(selected)) {
					browse_history.push_back(selected);
					directoryEnter(getPath(selected));
					selected = 0;
				} else {
					return true;
				}
			} else if (gmenu2x->input[SETTINGS] && allowSelectDirectory) {
				return true;
			} else if (gmenu2x->input[CANCEL] || gmenu2x->input[SETTINGS]) {
				if (!((gmenu2x->confStr["previewMode"] != "Backdrop") && !(preview.empty() || preview == "#")))
					return false; // close only if preview is empty.
				preview = "";
			} else if (gmenu2x->input[MANUAL]) {
				selected = (rand() % fileCount()) + dirCount();
			} else if (gmenu2x->input[MENU]) {
				contextMenu();
			}

			if (gmenu2x->input[UP] || gmenu2x->input[DOWN] || gmenu2x->input[LEFT] || gmenu2x->input[RIGHT] || gmenu2x->input[PAGEUP] || gmenu2x->input[PAGEDOWN]) {
				preview = getPreview(selected);
			}

		} while (!inputAction);
	}
}

void BrowseDialog::directoryEnter(string path) {
	gmenu2x->input.dropEvents(); // prevent passing input away
	gmenu2x->powerManager->clearTimer();

	this->description = path;
	buttons.clear();
	buttons.push_back({"skin:imgs/manual.png", _("Loading.. Please wait..")});

	drawDialog(gmenu2x->s);

	SDL_TimerID flipScreenTimer = SDL_AddTimer(500, GMenu2X::timerFlip, (void*)false);

	browse(path);
	onChangeDir();

	SDL_RemoveTimer(flipScreenTimer); flipScreenTimer = NULL;
	gmenu2x->powerManager->resetSuspendTimer();
}

const std::string BrowseDialog::getFileName(uint32_t i) {
	return getFile(i);
}
const std::string BrowseDialog::getParams(uint32_t i) {
	return "";
}
const std::string BrowseDialog::getPreview(uint32_t i) {
	string ext = getExt(i);
	if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" || ext == ".bmp") return getPath(i);
	return "";
}

void BrowseDialog::contextMenu() {
	vector<MenuOption> options;

	string ext = getExt(selected);

	customOptions(options);

	if (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" || ext == ".bmp")
		options.push_back((MenuOption){_("Set as wallpaper"), MakeDelegate(this, &BrowseDialog::setWallpaper)});

	if (path == "/media" && getFile(selected) != ".." && isDirectory(selected))
		options.push_back((MenuOption){_("Umount"), MakeDelegate(this, &BrowseDialog::umountDir)});

	if (path != CARD_ROOT)
		options.push_back((MenuOption){_F("Go to %s", CARD_ROOT), MakeDelegate(this, &BrowseDialog::exploreHome)});

	if (path != "/media")
		options.push_back((MenuOption){_F("Go to %s", "/media"), MakeDelegate(this, &BrowseDialog::exploreMedia)});

	if (isFile(selected))
		options.push_back((MenuOption){_("Delete"), MakeDelegate(this, &BrowseDialog::deleteFile)});

	MessageBox mb(gmenu2x, options);
}

void BrowseDialog::deleteFile() {
	MessageBox mb(gmenu2x, (string)_F("Delete %s", getFile(selected).c_str()) + "'\n" + _("THIS CAN'T BE UNDONE") + "\n" + _("Are you sure?"), "explorer.png");
	mb.setButton(MANUAL, _("Yes"));
	mb.setButton(CANCEL,  _("No"));
	if (mb.exec() != MANUAL) return;
	if (!unlink(getPath(selected).c_str())) {
		directoryEnter(path); // refresh
		sync();
	}
}

void BrowseDialog::umountDir() {
	string umount = "sync; umount -fl " + getPath(selected) + " && rm -r " + getPath(selected);
	system(umount.c_str());
	directoryEnter(path); // refresh
}

void BrowseDialog::exploreHome() {
	selected = 0;
	directoryEnter(CARD_ROOT);
}

void BrowseDialog::exploreMedia() {
	selected = 0;
	directoryEnter("/media");
}

void BrowseDialog::setWallpaper() {
	string src = getPath(selected);
	string dst = homePath + "/Wallpaper" + file_ext(src, true);
	if (file_copy(src, dst)) {
		gmenu2x->confStr["wallpaper"] = dst;
		gmenu2x->writeConfig();
		gmenu2x->setBackground(gmenu2x->bg, dst);
		this->exec();
	}
}
