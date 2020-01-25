#ifndef SPECTRUMMASTER_UI
#define SPECTRUMMASTER_UI

#include <ncpp/NotCurses.hh>

class UI {

public:

UI() {
}

~UI() {
  nc_.stop();
}

private:
ncpp::NotCurses nc_;

};

#endif
