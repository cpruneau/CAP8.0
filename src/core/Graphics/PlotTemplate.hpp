/* **********************************************************************
 * Copyright (C) 2019-2026, Claude Pruneau, Victor Gonzalez, Oveis Sheibani
 * All rights reserved.
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau
 *
 * *********************************************************************/
//! @file PlotTemplate.hpp
//! @brief PlotTemplate: a PlotHistos XML file made from the histograms of CAP files or of a workflow.
#ifndef CAP__PlotTemplate
#define CAP__PlotTemplate
#include <map>
#include <set>
#include <vector>
#include "String.hpp"
#include "XmlDocument.hpp"

namespace CAP
{
  class Folder;
  namespace GRAPHICS
  {

  //!
  //! @brief A PlotHistos file (see PlotSpec) made from a catalog of histograms: those of CAP files (addFolder() with
  //! their contents), or those the stages of a workflow write (addFolder() with the folders made by
  //! WorkflowHistograms, each stage an input).
  //!
  //! The histograms are catalogued by task (the first folder of their path, e.g. "Pair"), combination of filters (the
  //! folders between, e.g. "MB_PiP_PiM") and name (e.g. "R2_DyDphi"). Which names and combinations are plotted is
  //! chosen by task (all at first), by hand (setNameSelected(), setCombinationSelected()) or with patterns
  //! (select()). makeSpec() then lays them out: per task, one window (or several, of at most maxCanvases canvases)
  //! holding either one canvas per histogram name with a graph per combination, or one canvas per combination with
  //! a graph per histogram name; graphs of histograms of rank 1 can also overlay the combinations.
  //! @code
  //!   PlotTemplate t;
  //!   int input = t.addInput("pairs", "derived.cap");
  //!   t.addFolder(input, capFile.folder());
  //!   t.select({"Pair/*/n1_pt", "*R2_DyDphi"}, {"*_shft"});
  //!   t.makeSpec(options).write("plots.xml");
  //! @endcode
  //!
  class PlotTemplate
  {
  public:
  //! A histogram of the catalog
  struct Entry
  {
  int    input = 0;      //!< index of its input
  String path;           //!< path in its input, e.g. "Pair/MB_PiP_PiM/R2_DyDphi"
  String task;           //!< first folder ("" if none)
  String combination;    //!< folders between the task and the name ("" if none)
  String name;           //!< name of the histogram
  int    rank = 1;       //!< number of variables (1 or 2)
  };
  //! An input file
  struct Input { String id; String file; };
  //! How the histograms are laid out
  struct Options
  {
  //! One canvas per histogram name (a graph per combination), or one canvas per combination (a graph per name)
  enum Grouping { kByHistogram, kByCombination };
  Grouping grouping = kByHistogram;   //!< layout of the canvases
  bool overlay = false;               //!< histograms of rank 1 of a canvas overlaid in one graph (at most maxOverlay)
  int  maxOverlay = 8;                //!< histograms per graph when overlaid
  int  maxGraphs = 12;                //!< graphs per canvas (more: several canvases)
  int  maxCanvases = 6;               //!< canvases per window (more: several windows)
  String outputDirectory = "figures"; //!< directory of the outputs (Output element)
  String title = "Plots";             //!< title of the outputs
  bool printFiles = true;             //!< each canvas printed to <name>.pdf (attribute file)
  };

  //! @brief Empty catalog.
  PlotTemplate() { }
  //! @brief Add an input. @param id identifier @param file path of the CAP file @return index
  int addInput(const String & id, const String & file);
  //! @brief Set the file of an input. @param k index @param file path
  void setInputFile(int k, const String & file) { if (k >= 0 && k < int(_inputs.size())) _inputs[std::size_t(k)].file = file; }
  //! @brief Inputs. @return inputs
  const std::vector<Input> & inputs() const { return _inputs; }
  //!
  //! @brief Catalog the histograms (and other data points) of a folder tree, as found in input k. An entry of the same
  //! path already in the catalog is moved to this input (a later stage of a workflow writes the folders again,
  //! completed). Objects not decoded yet are decoded (to know their ranks).
  //! @param input index of the input @param top folder @return number of histograms catalogued
  //!
  int addFolder(int input, const Folder & top);
  //! @brief Catalog. @return entries, in the order found
  const std::vector<Entry> & entries() const { return _entries; }
  //! @brief Tasks, in the order found. @return names
  const std::vector<String> & tasks() const { return _tasks; }
  //! @brief Histogram names of a task, in the order found. @param task task @return names
  std::vector<String> names(const String & task) const;
  //! @brief Combinations of a task, in the order found. @param task task @return combinations
  std::vector<String> combinations(const String & task) const;
  //! @brief Rank of a histogram name of a task (that of its first entry). @param task task @param name name @return rank
  int rankOf(const String & task, const String & name) const;

  //! @name Selection (everything at first)
  //! @{
  bool isNameSelected(const String & task, const String & name) const { return !_unselectedNames.count(key(task, name)); }   //!< Whether a name is plotted
  bool isCombinationSelected(const String & task, const String & c) const { return !_unselectedCombinations.count(key(task, c)); }   //!< Whether a combination is plotted
  //! @brief Plot a name of a task, or not. @param task task @param name name @param v plotted
  void setNameSelected(const String & task, const String & name, bool v);
  //! @brief Plot a combination of a task, or not. @param task task @param c combination @param v plotted
  void setCombinationSelected(const String & task, const String & c, bool v);
  //! @brief Select all the names and combinations of a task, or none. @param task task ("" for all) @param v plotted
  void selectAll(const String & task, bool v);
  //!
  //! @brief Plot the histograms whose paths match one of the include patterns (all if there is none) and none of the
  //! exclude patterns: the names and combinations of the matching entries are selected, the others not. Patterns
  //! use * and ?; a pattern without '/' is matched to the name only. @param include patterns @param exclude patterns
  //!
  void select(const std::vector<String> & include, const std::vector<String> & exclude);
  //! @brief Whether an entry is plotted (its name and its combination selected). @param e entry @return true if so
  bool isSelected(const Entry & e) const { return isNameSelected(e.task, e.name) && isCombinationSelected(e.task, e.combination); }
  //! @brief Number of entries plotted. @return number
  int nSelected() const;
  //! @}

  //! @brief The PlotHistos document of the entries selected. @param options layout @return document
  XmlDocument makeSpec(const Options & options) const;
  //! @brief Whether a text matches a pattern of * and ?. @param text text @param pattern pattern @return true if so
  static bool matches(const String & text, const String & pattern);

  private:
  std::vector<Input> _inputs;   //!< inputs
  std::vector<Entry> _entries;  //!< catalog
  std::map<std::string, std::size_t> _byPath;   //!< index of an entry by its path
  std::vector<String> _tasks;   //!< tasks in the order found
  std::set<std::string> _unselectedNames;          //!< "task|name" not plotted
  std::set<std::string> _unselectedCombinations;   //!< "task|combination" not plotted
  static std::string key(const String & a, const String & b) { return a.str() + "|" + b.str(); }
  };

  } // namespace GRAPHICS
} // namespace CAP

#endif // !CAP__PlotTemplate
