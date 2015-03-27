#include "chat_show.h"
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


using std::string;

chat_show::chat_show(int y, int x, int height, int width):
  window(y, x, height, width),
  last_chat_len_(0)
{ }

chat_show::~chat_show()
{ }

int chat_show::chat(const char *name, const char *msg)
{
  string char_name(name);
  char_name.append(":");
  if (this->show(1, 1, char_name.c_str()) < 0
      || this->show(2, 2, msg) < 0)
    return -1;

  this->last_chat_len_ = strlen(msg);
  return 0;
}

void chat_show::clear()
{
  string empty(' ', this->width_-2);
  this->show(1, 1, empty.c_str());

  string message_empty(' ', this->last_chat_len_);
  this->show(2, 1, message_empty.c_str());
}
