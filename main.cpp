#include "source/rsdl.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include "source/map.hpp"





bool process_event(const Event &event, Window *win) {
  // static const string persons_names[] = {"Ghune", "Spartiate", "Athenian"};
  switch (event.get_type()) {
  case Event::QUIT:
    return false;
  case Event::LCLICK:
    // persons.push_back(Person(persons_names[rand() % 3], rand() % 2,
    //                          event.get_mouse_position()));
    // win->play_sound_effect("./example/assets/sound.wav");
    break;
  case Event::KEY_PRESS:
    if (event.get_pressed_key() == 'q')
      return false;
  default:;
  }
  return true;
}

void render(Window &win) {
  win.clear();
  // win.draw_img("example/assets/back.png");
  // for (vector<Person>::const_iterator person = persons.begin();
  //      person != persons.end(); person++)
  //   win.draw_img("example/assets/" + person->name + ".png",
  //                Rectangle(person->pos, person->pos + person->size), NULL_RECT,
  //                0, person->flipped);
  // win.draw_img("example/assets/cursor.png",
  //              Rectangle(get_current_mouse_position() - Point(15, 15), 30, 30));
  win.update_screen();
  delay(50);
}

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  Map map("../assets/maps/1/1.txt");
  try {
    Window win = Window();


    while (process_event(win.poll_for_event(), &win)) {
      render(win);
    }
  } catch (std::string exception) {
    std::cerr << "EXCEPTION: " << exception << std::endl;
    exit(1);
  }

  return 0;
}
