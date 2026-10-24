/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file CapGuiMain.cpp
//! @brief main() of bin/CapGui (CapGui.app on macOS).
//
// CapGui -- the CAP graphical interface.
//
//   CapGui [--config file.xml] [file or folder ...]
//
#include "CapApp.hpp"

//! @brief Start the graphical interface. @param argc number of arguments @param argv arguments
//! @return exit status
int main(int argc, char ** argv)
{
  CAP::GUI::CapApp application(argc, argv);
  return application.run();
}
