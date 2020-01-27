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

char32_t GetKey() {
  return nc_.getc(true);
}

private:
ncpp::NotCurses nc_;

};

#endif
