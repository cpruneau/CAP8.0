//
// CapGui -- the CAP graphical interface.
//
//   CapGui [--config file.xml] [file or folder ...]
//
#include "CapApp.hpp"

int main(int argc, char ** argv)
{
  CAP::GUI::CapApp application(argc, argv);
  return application.run();
}
