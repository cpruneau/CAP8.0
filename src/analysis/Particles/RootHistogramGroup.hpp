/* **********************************************************************
 * Copyright (C) 2019-2024, Claude Pruneau, Victor Gonzalez
 * All rights reserved.
 *
 * Based on the ROOT package and environment
 *
 * For the licensing terms see LICENSE.
 *
 * Author: Claude Pruneau,   04/01/2024
 *
 * *********************************************************************/
#ifndef CAP__RootHistogramGroup
#define CAP__RootHistogramGroup
#include <ostream>
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TProfile2D.h"
#include "Aliases.hpp"
#include "Configuration.hpp"
#include "ManagedObject.hpp"
#include "Exceptions.hpp"
#include "ClassDef.hpp"

namespace CAP
{

  class Task;

  //!
  //! @brief Group of ROOT histograms (TH1, TH2, TH3, TProfile, TProfile2D) owned by a task: creation, import from and
  //! saving to a ROOT file, scaling, and arithmetic on whole groups (sums, ratios) histogram by histogram.
  //!
  //! The histogram names are built from a base name (histogramBaseName()). Groups combined by the arithmetic functions
  //! must have the same histograms in the same order (sameSizeAs()); IncompatibleHistogramGroupException is thrown
  //! otherwise.
  //!
  class RootHistogramGroup
  :
  public ManagedObject
  {
  protected:

  //! Base of the histogram names.
  String _histogramBaseName;
  //! Task owning the group (not owned).
  Task * _parentTask;
  //! Histograms (owned).
  std::vector<TH1*> _objects;

  public:

  //! @brief Default constructor: empty group.
  RootHistogramGroup();
  //! @brief Copy constructor. @param source group to copy
  RootHistogramGroup(const RootHistogramGroup & source);
  //! @brief Copy assignment. @param rhs group to copy @return this group
  RootHistogramGroup & operator=(const RootHistogramGroup & rhs);
  //! @brief Add another group, histogram by histogram. @param rhs group @return this group
  RootHistogramGroup & operator+=(const RootHistogramGroup & rhs);
  //! @brief Subtract another group, histogram by histogram. @param rhs group @return this group
  RootHistogramGroup & operator-=(const RootHistogramGroup & rhs);
  //! @brief Destructor.
  virtual ~RootHistogramGroup() {}
  //! @brief Histograms. @return histograms
  const std::vector<TH1*> & histograms() const;
  //! @brief Histograms. @return histograms
  std::vector<TH1*> & histograms();
  //! @brief Configure the group. @param taskName name of the owning task @param objectType object type
  //! @param configuration configuration @param index index of the group
  virtual void configure(const String & taskName,
                         const String & objectType,
                         const Configuration & configuration,
                         unsigned int index);
  //  virtual void loadFrom(TFile & inputFile);
  //  virtual void loadFromAscii(std::ifstream & file __attribute__((unused))) {}
  //  virtual void loadFromAscii(std::ifstream & file1 __attribute__((unused)),
  //                                  std::ifstream & file2 __attribute__((unused))) {}
  //! @brief Write the histograms to a ROOT file. @param file file
  virtual void saveTo(TFile & file);
  //! @brief Reset the content of the histograms.
  virtual void reset();
  //! @brief Delete the histograms.
  virtual void clear();
  //! @brief Print the group.
  virtual void print() const;
  //! @brief Scale the histograms (profiles excluded) by a factor. @param a factor
  virtual void scaleHistograms(double a);
  //! @brief Task owning the group. @return task
  Task * parentTask() const  { return _parentTask; }
  //! @brief Name of the owning task. @return name
  const String parentName() const;
  //! @brief Task path of the owning task. @return path
  const String parentPathName() const;
  //! @brief Set the owning task. @param parentTask task
  void  setParentTask(Task * parentTask)
  {
  _parentTask = parentTask;
  }

  //! @brief Base of the histogram names. @return base name
  const String & baseName()
  {
  return _histogramBaseName;
  }

  //! @name Adding and creating histograms
  //! @{

  //! @brief Add a histogram, which the group then owns. @param h histogram @return h
  TH1 * append(TH1* h);
  //! @brief Add a 2D histogram. @param h histogram @return h
  TH2 * append(TH2* h);
  //! @brief Add a 3D histogram. @param h histogram @return h
  TH3 * append(TH3* h);
  //! @brief Add a profile. @param h profile @return h
  TProfile * append(TProfile* h);
  //! @brief Add a 2D profile. @param h profile @return h
  TProfile2D * append(TProfile2D* h);
  //! @brief Create and add a 1D histogram with uniform bins. @param name name @param n number of bins
  //! @param min_x lower edge @param max_x upper edge @param title_x x-axis title @param title_y y-axis title
  //! @param storageOption storage option (errors stored if 1) @return histogram
  TH1 * createHistogram(const String & name,
                        int n, double min_x, double max_x,
                        const String & title_x="",
                        const String & title_y="",
                        int storageOption=1);
  //! @brief Create and add a 1D histogram with variable bins. @param name name @param n number of bins
  //! @param bins bin edges (n+1) @param title_x x-axis title @param title_y y-axis title
  //! @param storageOption storage option @return histogram
  TH1 * createHistogram(const String & name,
                        int n, double * bins,
                        const String & title_x="",
                        const String & title_y="",
                        int storageOption=1);
  //! @brief Create and add a 2D histogram with uniform bins. @param name name
  //! @param n_x number of x bins @param min_x lower x edge @param max_x upper x edge
  //! @param n_y number of y bins @param rapidity_minimum lower y edge @param rapidity_maximum upper y edge
  //! @param title_x x-axis title @param title_y y-axis title @param title_z z-axis title @param storageOption storage option
  //! @return histogram
  TH2 * createHistogram(const String & name,
                        int n_x, double min_x, double max_x,
                        int n_y, double rapidity_minimum, double rapidity_maximum,
                        const String & title_x="",
                        const String & title_y="",
                        const String & title_z="",
                        int storageOption=1);
  //! @brief Create and add a 2D histogram with variable x bins. @param name name @param n_x number of x bins
  //! @param xbins x bin edges @param n_y number of y bins @param rapidity_minimum lower y edge
  //! @param rapidity_maximum upper y edge @param title_x x-axis title @param title_y y-axis title
  //! @param title_z z-axis title @param storageOption storage option @return histogram
  TH2 * createHistogram(const String & name,
                        int n_x, double* xbins, int n_y, double rapidity_minimum, double rapidity_maximum,
                        const String & title_x="",
                        const String & title_y="",
                        const String & title_z="",
                        int storageOption=1);
  //! @brief Create and add a 3D histogram with uniform bins (see the 2D version; n_z, z_min, z_max for z).
  TH3 * createHistogram(const String & name,
                        int n_x, double min_x, double max_x,
                        int n_y, double rapidity_minimum, double rapidity_maximum,
                        int n_z, double z_min, double z_max,
                        const String & title_x="",
                        const String & title_y="",
                        const String & title_z="",
                        int storageOption=1);
  //! @brief Create and add a profile with uniform bins. @param _name name @param n_x number of bins
  //! @param min_x lower edge @param max_x upper edge @param title_x x-axis title @param title_y y-axis title @return profile
  TProfile * createProfile(const String & _name,
                           int n_x,double min_x,double max_x,
                           const String & title_x,
                           const String & title_y);

  //! @brief Create and add a profile with variable bins. @param name name @param n_x number of bins
  //! @param bins bin edges @param title_x x-axis title @param title_y y-axis title @return profile
  TProfile * createProfile(const String & name,
                           int n_x,  double* bins,
                           const String & title_x,
                           const String & title_y);

  //! @brief Create and add a 2D profile with uniform bins. @param title name @param n_x number of x bins
  //! @param min_x lower x edge @param max_x upper x edge @param n_y number of y bins @param rapidity_minimum lower y edge
  //! @param rapidity_maximum upper y edge @param title_x x-axis title @param title_y y-axis title
  //! @param title_z z-axis title @return profile
  //! @}
  TProfile2D * createProfile(const String & title,
                             int n_x, double min_x, double max_x,
                             int n_y, double rapidity_minimum, double rapidity_maximum,
                             const String & title_x,
                             const String & title_y,
                             const String & title_z);

  //! @brief Add the histograms to a ROOT list (not owned by it). @param list list
  void addHistogramsToExtList(TList *list);

  //! @brief Whether another group has as many histograms. @param list group @return true if so
  bool sameSizeAs(const RootHistogramGroup & list) const;
  //! @brief Whether two other groups have as many histograms. @return true if so
  bool sameSizeAs(const RootHistogramGroup & list1, const RootHistogramGroup & list2) const;
  //! @brief Whether three other groups have as many histograms. @return true if so
  bool sameSizeAs(const RootHistogramGroup & list1, const RootHistogramGroup & list2, const RootHistogramGroup & list3) const;
  //! @brief Whether four other groups have as many histograms. @return true if so
  bool sameSizeAs(const RootHistogramGroup & list1, const RootHistogramGroup & list2, const RootHistogramGroup & list3, const RootHistogramGroup & list4) const;

  //
  //! @brief Load all the histograms of a ROOT file and add them to the group. @param inputFile file
  void loadList(TFile & inputFile);

  //! @name Arithmetic on groups (histogram by histogram)
  //! @{

  //! @brief sum += a1*list1. @throws IncompatibleHistogramGroupException for groups of different sizes
  virtual void sumHistogramGroups(double a1, const RootHistogramGroup & list1,
                                  RootHistogramGroup & sum);
  //! @brief sum += a1*list1 + a2*list2.
  virtual void sumHistogramGroups(double a1, const RootHistogramGroup & list1,
                                  double a2, const RootHistogramGroup & list2,
                                  RootHistogramGroup & sum);
  //! @brief sum += a1*list1 + a2*list2 + a3*list3.
  virtual void sumHistogramGroups(double a1, const RootHistogramGroup & list1,
                                  double a2, const RootHistogramGroup & list2,
                                  double a3, const RootHistogramGroup & list3,
                                  RootHistogramGroup & sum);
  //! @brief sum += a1*list1 + a2*list2 + a3*list3 + a4*list4.
  virtual void sumHistogramGroups(double a1, const RootHistogramGroup & list1,
                                  double a2, const RootHistogramGroup & list2,
                                  double a3, const RootHistogramGroup & list3,
                                  double a4, const RootHistogramGroup & list4,
                                  RootHistogramGroup & sum);

  //! @brief ratio = (a1*numerator)/(a2*denominator). @throws IncompatibleHistogramGroupException for groups of different sizes
  virtual void divideHistogramsLists(double a1, const RootHistogramGroup & numerator,
                                     double a2, const RootHistogramGroup & denominator,
                                     RootHistogramGroup & ratio);



  //! @brief Accumulate the squared differences with another group (variance estimate over sub-samples).
  //! @param list group @param sumWeights sum of the weights @param weight weight @param n number of sub-samples
  //! @}
  void squareDifferenceLists(const RootHistogramGroup & list,
                             double sumWeights, double weight, int n);

  //! @brief Copy of the group. @return new group (owned by the caller)
  RootHistogramGroup * clone() const;


  //! @brief Print the sizes of groups found incompatible.
  void printIncompatibleLists(const String & caller, const RootHistogramGroup & list1, const RootHistogramGroup & list2);
  void printIncompatibleLists(const String & caller, const RootHistogramGroup & list1, const RootHistogramGroup & list2, const RootHistogramGroup & list3);
  void printIncompatibleLists(const String & caller, const RootHistogramGroup & list1, const RootHistogramGroup & list2, const RootHistogramGroup & list3, const RootHistogramGroup & list4);
  void printIncompatibleLists(const String & caller, const RootHistogramGroup & list1, const RootHistogramGroup & list2, const RootHistogramGroup & list3, const RootHistogramGroup & list4, const RootHistogramGroup & list5);

  //! @brief Enable the storage of the sums of squared weights of the histograms (Sumw2).
  void sumw2All();
  //! @brief Divide the 1D and 2D histograms by their bin widths and multiply by a factor (profiles excluded).
  //! @param scale factor
  void scaleAllHistoByBinWidth(double scale);
  //! @brief Read a 1D histogram from a ROOT file and add it to the group. @param inputFile file @param histoName name @return histogram
  TH1 * importH1(TFile & inputFile,const String & histoName);
  //! @brief Read a 2D histogram from a ROOT file and add it to the group. @param inputFile file @param histoName name @return histogram
  TH2 * importH2(TFile & inputFile,const String & histoName);
  //! @brief Read a 3D histogram from a ROOT file and add it to the group. @param inputFile file @param histoName name @return histogram
  TH3 * importH3(TFile & inputFile,const String & histoName);
  //! @brief Read a profile from a ROOT file and add it to the group. @param inputFile file @param histoName name @return profile
  TProfile * importProfile(TFile & inputFile, const String & histoName);
  //! @brief Read a 2D profile from a ROOT file and add it to the group. @param inputFile file @param histoName name @return profile
  TProfile2D * importProfile2D(TFile & inputFile, const String & histoName);
  //! @brief Read from a ROOT file the histograms named as those of another group and add them to this group.
  //! @param inputFile file @param list group giving the names
  void loadInList(TFile & inputFile, RootHistogramGroup * list);

  //! @brief Number of histograms. @return number
  unsigned size() const
  {
  return _objects.size();
  }

  //! @brief Set the base of the histogram names. @param name base name
  void setHistogramBaseName(const String & name)
  {
  _histogramBaseName = name;
  }

  //! @brief Base of the histogram names. @return base name
  const String & histogramBaseName() const
  {
  return _histogramBaseName;
  }

  //! @brief Sum of two groups, histogram by histogram.
  friend RootHistogramGroup operator+(const RootHistogramGroup & lhs,const RootHistogramGroup & rhs);
  //! @brief Difference of two groups, histogram by histogram.
  friend RootHistogramGroup operator-(const RootHistogramGroup & lhs,const RootHistogramGroup & rhs);

  ClassDef(RootHistogramGroup,0)
  };

  RootHistogramGroup operator+(const RootHistogramGroup & lhs,const RootHistogramGroup & rhs);
  RootHistogramGroup operator-(const RootHistogramGroup & lhs,const RootHistogramGroup & rhs);


} // namespace CAP

#endif /* CAP__RootHistogramGroup */
